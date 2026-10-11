ConVar g_cvSteamApiKey;
ConVar g_cvSteamIDToolsBackend;
StringMap g_smProfileRequests;
StringMap g_smConversions;
int g_iNextProfileRequest = 1;
bool g_bProfilesStopping;

enum struct FamilyShareProfileRequest
{
	int accountId;
	int conversionId;
	int providerGeneration;
	Handle httpRequest;
	float deadline;
	char steamId64[32];
	char personaName[MAX_NAME_LENGTH];
	char profileUrl[256];
}

void Profiles_Init()
{
	g_cvSteamApiKey = CreateConVar("l4d2_familyshare_steam_api_key", "", "Steam Web API key for optional profile summaries.", FCVAR_PROTECTED | FCVAR_DONTRECORD);
	g_cvSteamIDToolsBackend = CreateConVar("l4d2_familyshare_steamidtools_backend", "1", "Use the optional SteamIDTools backend for profile identity enrichment.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_smProfileRequests = new StringMap();
	g_smConversions = new StringMap();
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

bool Profiles_bCanConvert()
{
	g_Runtime.canConvert = g_Runtime.hasSteamIDTools
		&& GetFeatureStatus(FeatureType_Native, "SteamIDTools_IsProviderReady") == FeatureStatus_Available
		&& GetFeatureStatus(FeatureType_Native, "SteamIDTools_RequestConversion") == FeatureStatus_Available;
	return g_cvSteamIDToolsBackend.BoolValue && g_Runtime.canConvert;
}

public void Frame_StartProfileRequest(any requestId)
{
	FamilyShareProfileRequest request;
	if (!Profiles_Get(requestId, request))
		return;
	SteamIDToolsProvider provider = SteamIDToolsProvider_Auto;
	if (Profiles_bCanConvert())
	{
		if (SteamIDTools_IsProviderReady(SteamIDToolsProvider_SteamWorks))
			provider = SteamIDToolsProvider_SteamWorks;
		else if (SteamIDTools_IsProviderReady(SteamIDToolsProvider_System2))
			provider = SteamIDToolsProvider_System2;
	}
	if (provider != SteamIDToolsProvider_Auto)
	{
		char account[16];
		FormatEx(account, sizeof(account), "%u", request.accountId);
		request.conversionId = SteamIDTools_RequestConversion(provider, API_AIDtoSID64, account, "familyshare_profile");
		if (request.conversionId > 0)
		{
			request.providerGeneration = g_Runtime.providerGeneration;
			char key[48];
			FormatEx(key, sizeof(key), "%d:%d", request.providerGeneration, request.conversionId);
			g_smConversions.SetValue(key, requestId);
			Profiles_Set(requestId, request);
			return;
		}
	}
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

public void SteamIDTools_OnRequestFinished(int conversionId, SteamIDToolsProvider provider, bool success,
	bool batch, const char[] endpoint, const char[] input, const char[] result, const char[] tag)
{
	if (batch || !StrEqual(tag, "familyshare_profile") || !StrEqual(endpoint, API_AIDtoSID64))
		return;
	char key[48];
	FormatEx(key, sizeof(key), "%d:%d", g_Runtime.providerGeneration, conversionId);
	int requestId;
	if (!g_smConversions.GetValue(key, requestId))
		return;
	g_smConversions.Remove(key);
	FamilyShareProfileRequest request;
	if (!Profiles_Get(requestId, request) || request.conversionId != conversionId)
		return;
	request.conversionId = 0;
	// A provider cannot change the identity attached to this request.
	if (success)
	{
		char resolved[32];
		strcopy(resolved, sizeof(resolved), result);
		TrimString(resolved);
		if (!StrEqual(resolved, request.steamId64))
			FamilyShare_LogDebug(g_Log, 2, "event=profile_conversion_mismatch request=%d", requestId);
	}
	Profiles_Set(requestId, request);
	Profiles_StartSummary(requestId, request);
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
	if (request.conversionId > 0)
	{
		FormatEx(key, sizeof(key), "%d:%d", request.providerGeneration, request.conversionId);
		g_smConversions.Remove(key);
	}
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
		if (GetEngineTime() >= request.deadline
			|| (request.conversionId > 0 && request.providerGeneration != g_Runtime.providerGeneration))
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
