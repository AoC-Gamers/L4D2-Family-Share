ConVar g_cvSteamApiKey;
StringMap g_smProfileRequests;
int g_iNextProfileRequest = 1;
bool g_bProfilesStopping;

enum struct FamilyShareProfileRequest
{
	int accountId;
	Handle httpRequest;
	float deadline;
	char steamId64[32];
	char personaName[MAX_NAME_LENGTH];
	char profileUrl[256];
}

void Profiles_Init()
{
	g_cvSteamApiKey = CreateConVar("l4d2_familyshare_steam_api_key", "", "Steam Web API key for optional profile summaries.", FCVAR_PROTECTED | FCVAR_DONTRECORD);
	g_smProfileRequests = new StringMap();
	// Requests intentionally survive map changes; deadlines use engine uptime.
	CreateTimer(1.0, Timer_ProfileRequests, _, TIMER_REPEAT);
}

int Native_RequestProfile(Handle plugin, int numParams)
{
	int accountId = GetNativeCell(1);
	if (accountId == 0 || g_bProfilesStopping)
		return 0;
	FamilyShareProfileRequest request;
	request.accountId = accountId;
	request.deadline = GetEngineTime() + 25.0;
	FamilyShare_AccountIdToSteamId64(accountId, request.steamId64, sizeof(request.steamId64));
	FormatEx(request.profileUrl, sizeof(request.profileUrl), "https://steamcommunity.com/profiles/%s", request.steamId64);
	int client = FamilyShare_FindConnectedAccount(accountId);
	if (client != 0)
		GetClientName(client, request.personaName, sizeof(request.personaName));
	int requestId = g_iNextProfileRequest++;
	char key[16];
	IntToString(requestId, key, sizeof(key));
	g_smProfileRequests.SetArray(key, request, sizeof(request));
	RequestFrame(Frame_StartProfileRequest, requestId);
	return requestId;
}

bool Profiles_Get(int requestId, FamilyShareProfileRequest request)
{
	char key[16];
	IntToString(requestId, key, sizeof(key));
	return g_smProfileRequests.GetArray(key, request, sizeof(request));
}

void Profiles_Set(int requestId, FamilyShareProfileRequest request)
{
	char key[16];
	IntToString(requestId, key, sizeof(key));
	g_smProfileRequests.SetArray(key, request, sizeof(request));
}

public void Frame_StartProfileRequest(any requestId)
{
	FamilyShareProfileRequest request;
	if (!Profiles_Get(requestId, request))
		return;
	Profiles_StartSummary(requestId, request);
}

void Profiles_StartSummary(int requestId, FamilyShareProfileRequest request)
{
	char apiKey[128];
	g_cvSteamApiKey.GetString(apiKey, sizeof(apiKey));
	if (apiKey[0] == '\0' || g_bProfilesStopping)
	{
		Profiles_Finish(requestId);
		return;
	}
	Handle http = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, "https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/");
	if (http == null)
	{
		Profiles_Finish(requestId);
		return;
	}
	if (!SteamWorks_SetHTTPRequestContextValue(http, requestId)
		|| !SteamWorks_SetHTTPRequestNetworkActivityTimeout(http, 10)
		|| !SteamWorks_SetHTTPRequestGetOrPostParameter(http, "key", apiKey)
		|| !SteamWorks_SetHTTPRequestGetOrPostParameter(http, "steamids", request.steamId64)
		|| !SteamWorks_SetHTTPCallbacks(http, OnProfileSummaryCompleted)
		|| !SteamWorks_SendHTTPRequest(http))
	{
		delete http;
		Profiles_Finish(requestId);
		return;
	}
	request.httpRequest = http;
	Profiles_Set(requestId, request);
}

public void OnProfileSummaryCompleted(Handle http, bool failure, bool successful, EHTTPStatusCode status, any requestId)
{
	FamilyShareProfileRequest request;
	if (!Profiles_Get(requestId, request))
		return;
	if (!failure && successful && status == k_EHTTPStatusCode200OK)
	{
		int size;
		if (SteamWorks_GetHTTPResponseBodySize(http, size) && size > 0 && size <= 65536)
		{
			char[] body = new char[size + 1];
			int written;
			bool truncated;
			if (SteamWorks_GetHTTPResponseBodyString(http, body, size + 1, written, truncated) && !truncated)
				Profiles_ParsePersonaName(body, request.personaName, sizeof(request.personaName));
		}
	}
	request.httpRequest = null;
	Profiles_Set(requestId, request);
	delete http;
	Profiles_Finish(requestId);
}

void Profiles_Finish(int requestId)
{
	FamilyShareProfileRequest request;
	if (!Profiles_Get(requestId, request))
		return;
	char key[48];
	IntToString(requestId, key, sizeof(key));
	g_smProfileRequests.Remove(key);
	if (request.httpRequest != null)
		delete request.httpRequest;
	Call_StartForward(g_fwdProfileResolved);
	Call_PushCell(requestId);
	Call_PushCell(request.accountId);
	Call_PushString(request.steamId64);
	Call_PushString(request.personaName);
	Call_PushString(request.profileUrl);
	Call_Finish();
}

public Action Timer_ProfileRequests(Handle timer)
{
	StringMapSnapshot snapshot = g_smProfileRequests.Snapshot();
	char key[16];
	for (int i = 0; i < snapshot.Length; i++)
	{
		snapshot.GetKey(i, key, sizeof(key));
		int requestId = StringToInt(key);
		FamilyShareProfileRequest request;
		if (!Profiles_Get(requestId, request))
			continue;
		if (GetEngineTime() >= request.deadline)
			Profiles_Finish(requestId);
	}
	delete snapshot;
	return Plugin_Continue;
}

void Profiles_Shutdown()
{
	g_bProfilesStopping = true;
	StringMapSnapshot snapshot = g_smProfileRequests.Snapshot();
	char key[16];
	for (int i = 0; i < snapshot.Length; i++)
	{
		snapshot.GetKey(i, key, sizeof(key));
		Profiles_Finish(StringToInt(key));
	}
	delete snapshot;
}

bool Profiles_ParsePersonaName(const char[] json, char[] output, int maxlen)
{
	int position = StrContains(json, "\"personaname\"", true);
	if (position < 0)
		return false;
	position += 13;
	while (json[position] == ' ' || json[position] == '\t' || json[position] == '\n' || json[position] == '\r')
		position++;
	if (json[position++] != ':')
		return false;
	while (json[position] == ' ' || json[position] == '\t' || json[position] == '\n' || json[position] == '\r')
		position++;
	if (json[position++] != '"')
		return false;
	char value[MAX_NAME_LENGTH];
	int length;
	bool escaped;
	for (; json[position] != '\0'; position++)
	{
		char ch = json[position];
		if (!escaped && ch == '"')
		{
			value[length] = '\0';
			strcopy(output, maxlen, value);
			return true;
		}
		if (!escaped && ch == '\\')
		{
			escaped = true;
			continue;
		}
		if (escaped && ch != '"' && ch != '\\' && ch != '/')
			return false; // Keep the captured name rather than display undecoded JSON.
		escaped = false;
		if (length < sizeof(value) - 1)
			value[length++] = ch;
	}
	return false;
}
