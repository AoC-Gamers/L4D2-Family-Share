#pragma semicolon 1
#pragma newdecls required

#include <colors>
#include <sourcemod>
#include <steamworks>

#undef REQUIRE_PLUGIN
#include <steamidtools>
#define REQUIRE_PLUGIN

#define FAMILYSHARE_TAG "[{olive}FamilyShare{default}]"
#define FAMILYSHARE_LOG "logs/FamilyShare.log"

enum FamilyShareSqlOperation
{
	FamilyShareSqlOperation_Insert = 0
}

ConVar g_cvEnforce;
ConVar g_cvAnnounce;
ConVar g_cvDebugSql;
ConVar g_cvSqlEnable;
ConVar g_cvSqlConfig;
ConVar g_cvSqlTable;
ConVar g_cvSteamApiKey;
ConVar g_cvSteamIDToolsBackend;
bool g_bSqlReady;
Database g_dbFamilyShare;
ArrayList g_aPendingFamilyShareSql;
Handle g_hFamilyShareSqlRetryTimer;
StringMap g_smPendingOwnerSid64Requests;

public Plugin myinfo =
{
	name        = "L4D2 Family Share",
	author      = "Lechuga",
	description = "Announce and optionally block players using shared copies of the game.",
	version     = "1.3.0",
	url         = ""
};

public void OnPluginStart()
{
	LoadTranslations("l4d2_familyshare.phrases");
	g_cvEnforce = CreateConVar("l4d2_familyshare_enforce", "1", "Block players using a shared copy of the game. 0 to only announce and log.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvAnnounce = CreateConVar("l4d2_familyshare_announce", "1", "Announce family share detections in chat.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvDebugSql = CreateConVar("l4d2_familyshare_sql_debug", "0", "Log SQL queue, flush and reconnect activity for family share.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvSqlEnable = CreateConVar("l4d2_familyshare_sql", "0", "Enable SQL logging for family share detections.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvSqlConfig = CreateConVar("l4d2_familyshare_sql_config", "default", "databases.cfg entry used for family share SQL logging.", FCVAR_NONE);
	g_cvSqlTable = CreateConVar("l4d2_familyshare_sql_table", "l4d2_familyshare", "SQL table used for family share logging.", FCVAR_NONE);
	g_cvSteamApiKey = CreateConVar("l4d2_familyshare_steam_api_key", "", "Steam Web API key used to resolve owner summaries.", FCVAR_PROTECTED | FCVAR_DONTRECORD);
	g_cvSteamIDToolsBackend = CreateConVar("l4d2_familyshare_steamidtools_backend", "1", "Use SteamIDTools backend to resolve owner SteamID64 when available.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvSqlEnable.AddChangeHook(OnFamilyShareSqlSettingsChanged);
	g_cvSqlConfig.AddChangeHook(OnFamilyShareSqlSettingsChanged);
	g_aPendingFamilyShareSql = new ArrayList();
	g_smPendingOwnerSid64Requests = new StringMap();

	AutoExecConfig(true, "l4d2_familyshare");
}

public void OnConfigsExecuted()
{
	ConnectFamilyShareDatabase();
}

public void OnPluginEnd()
{
	CancelFamilyShareSqlRetry();
	ClearPendingFamilyShareSql();

	if (g_dbFamilyShare != null)
		delete g_dbFamilyShare;
	if (g_smPendingOwnerSid64Requests != null)
		delete g_smPendingOwnerSid64Requests;
}

void ConnectFamilyShareDatabase()
{
	CancelFamilyShareSqlRetry();
	g_bSqlReady = false;

	if (g_dbFamilyShare != null)
	{
		delete g_dbFamilyShare;
		g_dbFamilyShare = null;
	}

	if (!g_cvSqlEnable.BoolValue)
	{
		ClearPendingFamilyShareSql();
		g_aPendingFamilyShareSql = new ArrayList();
		LogFamilyShareSqlDebug("sql disabled pending cleared");
		return;
	}

	char configName[64];
	g_cvSqlConfig.GetString(configName, sizeof(configName));
	if (!configName[0])
		return;

	LogFamilyShareSqlDebug("connecting config=%s", configName);
	Database.Connect(OnFamilyShareDatabaseConnected, configName);
}

public void OnFamilyShareDatabaseConnected(Database db, const char[] error, any data)
{
	if (db == null)
	{
		LogError("[FamilyShare] SQL connection failed: %s", error);
		ScheduleFamilyShareSqlRetry();
		return;
	}

	g_dbFamilyShare = db;
	g_bSqlReady = true;
	LogFamilyShareSqlDebug("connected pending=%d", g_aPendingFamilyShareSql != null ? g_aPendingFamilyShareSql.Length : 0);
	FlushPendingFamilyShareSql();
}

public void OnFamilyShareSqlSettingsChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	ConnectFamilyShareDatabase();
}

void ScheduleFamilyShareSqlRetry()
{
	if (g_hFamilyShareSqlRetryTimer != null || !g_cvSqlEnable.BoolValue)
		return;

	LogFamilyShareSqlDebug("reconnect scheduled delay=30");
	g_hFamilyShareSqlRetryTimer = CreateTimer(30.0, Timer_RetryFamilyShareDatabase, _, TIMER_FLAG_NO_MAPCHANGE);
}

void CancelFamilyShareSqlRetry()
{
	if (g_hFamilyShareSqlRetryTimer == null)
		return;

	delete g_hFamilyShareSqlRetryTimer;
	g_hFamilyShareSqlRetryTimer = null;
}

public Action Timer_RetryFamilyShareDatabase(Handle timer)
{
	g_hFamilyShareSqlRetryTimer = null;
	LogFamilyShareSqlDebug("reconnect retry");
	ConnectFamilyShareDatabase();
	return Plugin_Stop;
}

void LogFamilyShareSqlDebug(const char[] message, any ...)
{
	if (g_cvDebugSql == null || !g_cvDebugSql.BoolValue)
		return;

	char buffer[256];
	VFormat(buffer, sizeof(buffer), message, 2);
	LogMessage("[FamilyShare][sql] %s", buffer);
}

bool GetFamilyShareSteamApiKey(char[] buffer, int maxlen)
{
	if (g_cvSteamApiKey == null)
		return false;

	g_cvSteamApiKey.GetString(buffer, maxlen);
	return buffer[0] != '\0';
}

stock int GetClientOfAccountIdUnvalidated(int accountId)
{
	if (accountId <= 0)
		return -1;

	for (int client = 1; client <= MaxClients; client++)
	{
		if (!IsClientConnected(client))
			continue;

		if (GetSteamAccountID(client, false) == accountId)
			return client;
	}

	return -1;
}

void GetOwnerDisplayName(int ownerAccountId, char[] buffer, int maxlen)
{
	int ownerClient = GetClientOfAccountIdUnvalidated(ownerAccountId);
	if (IsValidClientIndex(ownerClient))
	{
		GetClientName(ownerClient, buffer, maxlen);
		return;
	}

	Format(buffer, maxlen, "%T", "UnknownOwner", LANG_SERVER);
}

void BuildFamilyShareLogPath(char[] buffer, int maxlen)
{
	BuildPath(Path_SM, buffer, maxlen, FAMILYSHARE_LOG);
}

bool GetClientSteamId64(int client, char[] buffer, int maxlen)
{
	if (!IsValidClientIndex(client))
		return false;

	return GetClientAuthId(client, AuthId_SteamID64, buffer, maxlen);
}

bool AccountIDToSteamID2String(int accountId, char[] buffer, int maxlen)
{
	if (accountId <= 0)
		return false;

	int y = accountId & 1;
	int z = accountId >> 1;
	Format(buffer, maxlen, "STEAM_1:%d:%d", y, z);
	return true;
}

bool AccountIDToSteamID3String(int accountId, char[] buffer, int maxlen)
{
	if (accountId <= 0)
		return false;

	Format(buffer, maxlen, "[U:1:%d]", accountId);
	return true;
}

bool BuildProfileURL(const char[] profileId, char[] buffer, int maxlen)
{
	if (profileId[0] == '\0')
		return false;

	Format(buffer, maxlen, "https://steamcommunity.com/profiles/%s", profileId);
	return true;
}

void LogFamilyShareDetection(const char[] playerName, int borrowerAccountId, const char[] borrowerSteamId64, const char[] ownerName, const char[] ownerSteamId64, const char[] ownerProfileUrl)
{
	char logFile[PLATFORM_MAX_PATH];
	BuildFamilyShareLogPath(logFile, sizeof(logFile));

	LogToFile(logFile, "[Borrower Name: %s | Borrower AID: %d | Borrower SID64: %s | Owner Name: %s | Owner SID64: %s | Owner Profile: %s]",
		playerName,
		borrowerAccountId,
		borrowerSteamId64[0] != '\0' ? borrowerSteamId64 : "UNKNOWN",
		ownerName,
		ownerSteamId64[0] != '\0' ? ownerSteamId64 : "UNKNOWN",
		ownerProfileUrl[0] != '\0' ? ownerProfileUrl : "UNKNOWN");
}

void InsertFamilyShareSqlEvent(const char[] playerName, int borrowerAccountId, const char[] borrowerSid64, const char[] ownerName, const char[] ownerSid64, bool enforced)
{
	if (!g_cvSqlEnable.BoolValue)
		return;

	if (!g_bSqlReady || g_dbFamilyShare == null)
	{
		LogFamilyShareSqlDebug("queue insert borrower=%d owner_sid64=%s", borrowerAccountId, ownerSid64);
		QueuePendingFamilyShareInsert(playerName, borrowerAccountId, borrowerSid64, ownerName, ownerSid64, enforced);
		return;
	}

	char tableName[64];
	char escapedPlayerName[2 * MAX_NAME_LENGTH + 1];
	char escapedBorrowerSid64[65];
	char escapedOwnerName[2 * MAX_NAME_LENGTH + 1];
	char escapedOwnerSid64[65];
	char borrowerSid64Sql[96];
	char ownerSid64Sql[96];
	char query[2048];

	g_cvSqlTable.GetString(tableName, sizeof(tableName));
	g_dbFamilyShare.Escape(playerName, escapedPlayerName, sizeof(escapedPlayerName));
	g_dbFamilyShare.Escape(borrowerSid64, escapedBorrowerSid64, sizeof(escapedBorrowerSid64));
	g_dbFamilyShare.Escape(ownerName, escapedOwnerName, sizeof(escapedOwnerName));
	g_dbFamilyShare.Escape(ownerSid64, escapedOwnerSid64, sizeof(escapedOwnerSid64));

	if (escapedBorrowerSid64[0] != '\0')
		Format(borrowerSid64Sql, sizeof(borrowerSid64Sql), "'%s'", escapedBorrowerSid64);
	else
		strcopy(borrowerSid64Sql, sizeof(borrowerSid64Sql), "NULL");

	if (escapedOwnerSid64[0] != '\0')
		Format(ownerSid64Sql, sizeof(ownerSid64Sql), "'%s'", escapedOwnerSid64);
	else
		strcopy(ownerSid64Sql, sizeof(ownerSid64Sql), "NULL");

	Format(query, sizeof(query),
		"INSERT INTO `%s` (`borrower_name`, `borrower_accountid`, `borrower_steamid64`, `owner_name`, `owner_steamid64`, `enforced`) VALUES ('%s', %d, %s, '%s', %s, %d);",
		tableName,
		escapedPlayerName,
		borrowerAccountId,
		borrowerSid64Sql,
		escapedOwnerName,
		ownerSid64Sql,
		enforced ? 1 : 0);

	LogFamilyShareSqlDebug("insert borrower=%d owner_sid64=%s", borrowerAccountId, ownerSid64);
	g_dbFamilyShare.Query(OnFamilyShareSqlQueryFinished, query);
}

public void OnFamilyShareSqlQueryFinished(Database db, DBResultSet results, const char[] error, any data)
{
	if (error[0] != '\0')
		LogError("[FamilyShare] SQL query failed: %s", error);
}

void QueuePendingFamilyShareInsert(const char[] playerName, int borrowerAccountId, const char[] borrowerSid64, const char[] ownerName, const char[] ownerSid64, bool enforced)
{
	DataPack pack = new DataPack();
	pack.WriteCell(FamilyShareSqlOperation_Insert);
	pack.WriteString(playerName);
	pack.WriteCell(borrowerAccountId);
	pack.WriteString(borrowerSid64);
	pack.WriteString(ownerName);
	pack.WriteString(ownerSid64);
	pack.WriteCell(enforced ? 1 : 0);
	g_aPendingFamilyShareSql.Push(pack);
}

void FlushPendingFamilyShareSql()
{
	if (!g_bSqlReady || g_dbFamilyShare == null)
		return;

	int count = g_aPendingFamilyShareSql.Length;
	LogFamilyShareSqlDebug("flush count=%d", count);
	for (int i = 0; i < count; i++)
	{
		DataPack pack = view_as<DataPack>(g_aPendingFamilyShareSql.Get(i));
		if (pack == null)
			continue;

		pack.Reset();
		pack.ReadCell();

		char playerName[MAX_NAME_LENGTH];
		char borrowerSid64[32];
		char ownerName[MAX_NAME_LENGTH];
		char ownerSid64[32];

		pack.ReadString(playerName, sizeof(playerName));
		int borrowerAccountId = pack.ReadCell();
		pack.ReadString(borrowerSid64, sizeof(borrowerSid64));
		pack.ReadString(ownerName, sizeof(ownerName));
		pack.ReadString(ownerSid64, sizeof(ownerSid64));
		bool enforced = pack.ReadCell() == 1;

		InsertFamilyShareSqlEvent(playerName, borrowerAccountId, borrowerSid64, ownerName, ownerSid64, enforced);

		delete pack;
	}

	g_aPendingFamilyShareSql.Clear();
}

void ClearPendingFamilyShareSql()
{
	if (g_aPendingFamilyShareSql == null)
		return;

	int count = g_aPendingFamilyShareSql.Length;
	for (int i = 0; i < count; i++)
	{
		DataPack pack = view_as<DataPack>(g_aPendingFamilyShareSql.Get(i));
		if (pack != null)
			delete pack;
	}

	delete g_aPendingFamilyShareSql;
	g_aPendingFamilyShareSql = null;
}

void AnnounceFamilyShare(const char[] playerName, const char[] borrowerSteamId64, const char[] ownerName, int ownerAccountId, const char[] ownerSteamId64, const char[] ownerProfileUrl)
{
	if (!g_cvAnnounce.BoolValue)
		return;

	CPrintToChatAll("%t %t", "Tag", "SharedCopyDetected", playerName, borrowerSteamId64);

	if (ownerSteamId64[0] != '\0' && ownerProfileUrl[0] != '\0')
	{
		CPrintToChatAll("%t %t %t", "Tag", "OwnerProfileResolvedLabel", ownerName, "OwnerProfileUrl", ownerProfileUrl);
	}
	else
	{
		char ownerSteamId3[32];
		char ownerFallbackProfileUrl[256];
		ownerFallbackProfileUrl[0] = '\0';

		if (AccountIDToSteamID3String(ownerAccountId, ownerSteamId3, sizeof(ownerSteamId3)))
			BuildProfileURL(ownerSteamId3, ownerFallbackProfileUrl, sizeof(ownerFallbackProfileUrl));

		if (ownerFallbackProfileUrl[0] != '\0')
			CPrintToChatAll("%t %t %t", "Tag", "OwnerProfileLabel", "OwnerProfileUrl", ownerFallbackProfileUrl);
		else
			CPrintToChatAll("%t %t", "Tag", "OwnerProfileLabel");
	}
}

bool ParseJsonStringValue(const char[] json, const char[] key, char[] buffer, int maxlen)
{
	char needle[64];
	Format(needle, sizeof(needle), "\"%s\":\"", key);
	int start = StrContains(json, needle, false);
	if (start == -1)
		return false;

	start += strlen(needle);
	int out = 0;
	bool escaped = false;

	for (int i = start; json[i] != '\0'; i++)
	{
		char ch = json[i];
		if (escaped)
		{
			if (out < maxlen - 1)
				buffer[out++] = ch;
			escaped = false;
			continue;
		}

		if (ch == '\\')
		{
			escaped = true;
			continue;
		}

		if (ch == '"')
			break;

		if (out < maxlen - 1)
			buffer[out++] = ch;
	}

	buffer[out] = '\0';
	return out > 0;
}

SteamIDToolsProvider GetReadySteamIDToolsProvider()
{
	if (g_cvSteamIDToolsBackend == null || !g_cvSteamIDToolsBackend.BoolValue)
		return SteamIDToolsProvider_Auto;

	if (!SteamIDTools_IsLibraryAvailable())
		return SteamIDToolsProvider_Auto;

	if (SteamIDTools_IsProviderReady(SteamIDToolsProvider_SteamWorks))
		return SteamIDToolsProvider_SteamWorks;

	if (SteamIDTools_IsProviderReady(SteamIDToolsProvider_System2))
		return SteamIDToolsProvider_System2;

	return SteamIDToolsProvider_Auto;
}

DataPack CloneFamilySharePackWithOwner(DataPack sourcePack, const char[] ownerSteamId64, const char[] ownerProfileUrl)
{
	if (sourcePack == null)
		return null;

	sourcePack.Reset();

	char playerName[MAX_NAME_LENGTH];
	char borrowerSid64[32];
	char ownerName[MAX_NAME_LENGTH];
	char ownerSid2[32];
	char ignoredOwnerSid64[32];
	char ignoredOwnerProfileUrl[256];

	sourcePack.ReadString(playerName, sizeof(playerName));
	int borrowerAccountId = sourcePack.ReadCell();
	sourcePack.ReadString(borrowerSid64, sizeof(borrowerSid64));
	sourcePack.ReadString(ownerName, sizeof(ownerName));
	sourcePack.ReadString(ownerSid2, sizeof(ownerSid2));
	sourcePack.ReadString(ignoredOwnerSid64, sizeof(ignoredOwnerSid64));
	sourcePack.ReadString(ignoredOwnerProfileUrl, sizeof(ignoredOwnerProfileUrl));
	bool enforced = sourcePack.ReadCell() == 1;

	DataPack newPack = new DataPack();
	newPack.WriteString(playerName);
	newPack.WriteCell(borrowerAccountId);
	newPack.WriteString(borrowerSid64);
	newPack.WriteString(ownerName);
	newPack.WriteString(ownerSid2);
	newPack.WriteString(ownerSteamId64);
	newPack.WriteString(ownerProfileUrl);
	newPack.WriteCell(enforced ? 1 : 0);
	return newPack;
}

void ProcessPendingFamilyShareDetection(DataPack pack, const char[] resolvedOwnerName = "")
{
	if (pack == null)
		return;

	pack.Reset();

	char playerName[MAX_NAME_LENGTH];
	char borrowerSid64[32];
	char ownerName[MAX_NAME_LENGTH];
	char ownerSid2[32];
	char ownerSid64[32];
	char ownerProfileUrl[256];

	pack.ReadString(playerName, sizeof(playerName));
	int borrowerAccountId = pack.ReadCell();
	pack.ReadString(borrowerSid64, sizeof(borrowerSid64));
	pack.ReadString(ownerName, sizeof(ownerName));
	pack.ReadString(ownerSid2, sizeof(ownerSid2));
	pack.ReadString(ownerSid64, sizeof(ownerSid64));
	pack.ReadString(ownerProfileUrl, sizeof(ownerProfileUrl));
	bool enforced = pack.ReadCell() == 1;

	if (resolvedOwnerName[0] != '\0')
		strcopy(ownerName, sizeof(ownerName), resolvedOwnerName);

	int ownerAccountId = 0;
	if (ownerSid2[0] != '\0')
	{
		char sidParts[3][16];
		if (ExplodeString(ownerSid2, ":", sidParts, sizeof(sidParts), sizeof(sidParts[])) == 3)
			ownerAccountId = StringToInt(sidParts[2]) * 2 + StringToInt(sidParts[1]);
	}

	if (ownerProfileUrl[0] == '\0' && ownerAccountId > 0)
	{
		char ownerSteamId3[32];
		if (AccountIDToSteamID3String(ownerAccountId, ownerSteamId3, sizeof(ownerSteamId3)))
			BuildProfileURL(ownerSteamId3, ownerProfileUrl, sizeof(ownerProfileUrl));
	}

	LogFamilyShareDetection(playerName, borrowerAccountId, borrowerSid64, ownerName, ownerSid64, ownerProfileUrl);
	InsertFamilyShareSqlEvent(playerName, borrowerAccountId, borrowerSid64, ownerName, ownerSid64, enforced);
	AnnounceFamilyShare(playerName, borrowerSid64, ownerName, ownerAccountId, ownerSid64, ownerProfileUrl);
}

bool StartOwnerSummaryRequest(DataPack pack, const char[] ownerSteamId64)
{
	if (ownerSteamId64[0] == '\0')
		return false;

	char apiKey[128];
	if (!GetFamilyShareSteamApiKey(apiKey, sizeof(apiKey)))
		return false;

	Handle hRequest = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, "https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/");
	if (hRequest == INVALID_HANDLE)
		return false;

	SteamWorks_SetHTTPRequestContextValue(hRequest, pack);
	SteamWorks_SetHTTPRequestNetworkActivityTimeout(hRequest, 10);
	SteamWorks_SetHTTPRequestGetOrPostParameter(hRequest, "key", apiKey);
	SteamWorks_SetHTTPRequestGetOrPostParameter(hRequest, "steamids", ownerSteamId64);
	SteamWorks_SetHTTPCallbacks(hRequest, OnOwnerSummaryRequestCompleted);

	if (!SteamWorks_SendHTTPRequest(hRequest))
	{
		delete hRequest;
		return false;
	}

	return true;
}

bool StartOwnerSteamId64Resolution(DataPack pack, int ownerAccountId)
{
	if (pack == null || ownerAccountId <= 0 || g_smPendingOwnerSid64Requests == null)
		return false;

	SteamIDToolsProvider provider = GetReadySteamIDToolsProvider();
	if (provider == SteamIDToolsProvider_Auto)
		return false;

	char input[16];
	IntToString(ownerAccountId, input, sizeof(input));

	int requestId = SteamIDTools_RequestConversion(provider, API_AIDtoSID64, input, "familyshare_owner_sid64");
	if (requestId <= 0)
		return false;

	char key[16];
	IntToString(requestId, key, sizeof(key));
	g_smPendingOwnerSid64Requests.SetValue(key, pack);
	return true;
}

public void OnOwnerSummaryRequestCompleted(Handle hRequest, bool bFailure, bool bRequestSuccessful, EHTTPStatusCode eStatusCode, any data1)
{
	DataPack pack = view_as<DataPack>(data1);
	char ownerName[MAX_NAME_LENGTH];
	ownerName[0] = '\0';

	if (!bFailure && bRequestSuccessful && eStatusCode == k_EHTTPStatusCode200OK)
	{
		int size = 0;
		if (SteamWorks_GetHTTPResponseBodySize(hRequest, size) && size > 0)
		{
			char[] body = new char[size + 1];
			int written = 0;
			bool truncated = false;

			if (SteamWorks_GetHTTPResponseBodyString(hRequest, body, size + 1, written, truncated) && !truncated)
			{
				ParseJsonStringValue(body, "personaname", ownerName, sizeof(ownerName));
			}
		}
	}

	ProcessPendingFamilyShareDetection(pack, ownerName);

	if (hRequest != null)
		delete hRequest;
	if (pack != null)
		delete pack;
}

public void SteamIDTools_OnRequestFinished(int iRequestId, SteamIDToolsProvider provider, bool bSuccess, bool bBatch, const char[] szEndpoint, const char[] szInput, const char[] szResult, const char[] szTag)
{
	if (bBatch || !StrEqual(szTag, "familyshare_owner_sid64") || !StrEqual(szEndpoint, API_AIDtoSID64))
		return;

	if (g_smPendingOwnerSid64Requests == null)
		return;

	char key[16];
	IntToString(iRequestId, key, sizeof(key));

	int packValue = 0;
	if (!g_smPendingOwnerSid64Requests.GetValue(key, packValue))
		return;

	g_smPendingOwnerSid64Requests.Remove(key);

	DataPack originalPack = view_as<DataPack>(packValue);
	if (originalPack == null)
		return;

	char ownerSteamId64[32];
	char ownerProfileUrl[256];
	ownerSteamId64[0] = '\0';
	ownerProfileUrl[0] = '\0';

	if (bSuccess && szResult[0] != '\0')
	{
		strcopy(ownerSteamId64, sizeof(ownerSteamId64), szResult);
		TrimString(ownerSteamId64);
	BuildProfileURL(ownerSteamId64, ownerProfileUrl, sizeof(ownerProfileUrl));
	}

	DataPack nextPack = CloneFamilySharePackWithOwner(originalPack, ownerSteamId64, ownerProfileUrl);
	delete originalPack;

	if (nextPack == null)
		return;

	if (!StartOwnerSummaryRequest(nextPack, ownerSteamId64))
	{
		ProcessPendingFamilyShareDetection(nextPack);
		delete nextPack;
	}
}

public void SteamWorks_OnValidateClient(int ownerauthid, int authid)
{
	int client = GetClientOfAccountIdUnvalidated(authid);
	if (ownerauthid == authid)
		return;

	if (!IsValidClientIndex(client))
	{
		LogMessage("[FamilyShare] Unable to resolve borrower client for accountid %d owner %d during validation.", authid, ownerauthid);
		return;
	}

	char playerName[MAX_NAME_LENGTH];
	char borrowerSid64[32];
	char ownerName[MAX_NAME_LENGTH];
	char ownerSid2[32];
	char ownerSteamId64[32];
	char ownerProfileUrl[256];
	char kickMessage[128];

	GetClientName(client, playerName, sizeof(playerName));
	GetOwnerDisplayName(ownerauthid, ownerName, sizeof(ownerName));
	if (!AccountIDToSteamID2String(ownerauthid, ownerSid2, sizeof(ownerSid2)))
		ownerSid2[0] = '\0';

	if (!GetClientSteamId64(client, borrowerSid64, sizeof(borrowerSid64)))
		borrowerSid64[0] = '\0';

	ownerSteamId64[0] = '\0';
	ownerProfileUrl[0] = '\0';

	DataPack pack = new DataPack();
	pack.WriteString(playerName);
	pack.WriteCell(authid);
	pack.WriteString(borrowerSid64);
	pack.WriteString(ownerName);
	pack.WriteString(ownerSid2);
	pack.WriteString(ownerSteamId64);
	pack.WriteString(ownerProfileUrl);
	pack.WriteCell(g_cvEnforce.BoolValue ? 1 : 0);

	if (!StartOwnerSteamId64Resolution(pack, ownerauthid))
	{
		ProcessPendingFamilyShareDetection(pack);
		delete pack;
	}

	if (!g_cvEnforce.BoolValue)
		return;

	Format(kickMessage, sizeof(kickMessage), "%T", "KickClient", client);
	KickClient(client, kickMessage);
}
