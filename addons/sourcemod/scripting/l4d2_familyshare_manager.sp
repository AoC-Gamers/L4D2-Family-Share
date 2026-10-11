#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <colors>
#undef REQUIRE_PLUGIN
#include <l4d2_familyshare_core>
#define REQUIRE_PLUGIN
#include <l4d2_familyshare_stock>

enum struct FamilyShareManagerRuntimeState
{
	bool lateLoad;
	bool hasCore;
	bool canQueryCore;
	bool canRequestProfile;
	int coreGeneration;

	void Reset()
	{
		this.lateLoad = false;
		this.hasCore = false;
		this.canQueryCore = false;
		this.canRequestProfile = false;
		this.coreGeneration = 0;
	}

	void DetectLibraries()
	{
		this.hasCore = LibraryExists(L4D2_FAMILYSHARE_CORE_LIBRARY);
		if (!this.hasCore)
		{
			this.canQueryCore = false;
			this.canRequestProfile = false;
		}
	}

	void SetLibraryAvailability(const char[] name, bool available)
	{
		if (!StrEqual(name, L4D2_FAMILYSHARE_CORE_LIBRARY))
			return;
		this.hasCore = available;
		if (!available)
		{
			this.canQueryCore = false;
			this.canRequestProfile = false;
			this.coreGeneration++;
		}
	}
}

FamilyShareManagerRuntimeState g_Runtime;
FamilyShareRuntimeLog g_Log;

enum struct FamilyShareDetection
{
	int historyId;
	int coreGeneration;
	int clientSerial;
	int borrowerAccountId;
	int ownerAccountId;
	bool allowed;
	bool enforced;
	char borrowerName[MAX_NAME_LENGTH];
	char borrowerSteamId64[32];
	char ownerName[MAX_NAME_LENGTH];
	char ownerSteamId64[32];
	char ownerProfileUrl[256];
	float profileDeadline;
}

ConVar g_cvEnforce;
ConVar g_cvAnnounce;
StringMap g_smPendingDetections;
bool g_bClientEnforced[MAXPLAYERS + 1];
bool g_bManagerStopping;

#include "l4d2_familyshare_manager/policy.sp"
#include "l4d2_familyshare_manager/storage.sp"

public Plugin myinfo =
{
	name = "L4D2 Family Share Manager",
	author = "Lechuga",
	description = "Borrower exceptions, announcements, enforcement and detection history.",
	version = "2.0.0",
	url = "https://github.com/AoC-Gamers/L4D2-Family-Share"
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int errMax)
{
	g_Runtime.Reset();
	g_Runtime.lateLoad = late;
	RegPluginLibrary("l4d2_familyshare_manager");
	CreateNative("L4D2FamilyShareManager_IsAccountAllowed", Native_IsAccountAllowed);
	CreateNative("L4D2FamilyShareManager_IsClientEnforced", Native_IsClientEnforced);
	return APLRes_Success;
}

public void OnPluginStart()
{
	LoadTranslations("l4d2_familyshare.phrases");
	g_cvEnforce = CreateConVar("l4d2_familyshare_enforce", "1", "Kick detected borrowers unless their own AccountID is allowed.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvAnnounce = CreateConVar("l4d2_familyshare_announce", "1", "Announce Family Sharing detections in chat.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_Log.Init("manager", "1");
	g_smPendingDetections = new StringMap();
	Policy_Init();
	Storage_Init();
	CreateTimer(1.0, Timer_PendingDetections, _, TIMER_REPEAT);
	AutoExecConfig(true, "l4d2_familyshare_manager");
}

public void OnConfigsExecuted()
{
	Policy_Reload();
	Storage_Reconcile();
}

public void OnAllPluginsLoaded()
{
	g_Runtime.DetectLibraries();
}

public void OnLibraryAdded(const char[] name)
{
	g_Runtime.SetLibraryAvailability(name, true);
}

public void OnLibraryRemoved(const char[] name)
{
	g_Runtime.SetLibraryAvailability(name, false);
}

void RefreshCoreCapabilities()
{
	g_Runtime.canQueryCore = g_Runtime.hasCore
		&& GetFeatureStatus(FeatureType_Native, "L4D2FamilyShareCore_IsClientDetected") == FeatureStatus_Available
		&& GetFeatureStatus(FeatureType_Native, "L4D2FamilyShareCore_GetClientBorrowerAccountId") == FeatureStatus_Available
		&& GetFeatureStatus(FeatureType_Native, "L4D2FamilyShareCore_GetClientOwnerAccountId") == FeatureStatus_Available;
	g_Runtime.canRequestProfile = g_Runtime.hasCore
		&& GetFeatureStatus(FeatureType_Native, "L4D2FamilyShareCore_RequestProfile") == FeatureStatus_Available;
}

public void OnClientConnected(int client)
{
	g_bClientEnforced[client] = false;
}

public void OnClientDisconnect(int client)
{
	g_bClientEnforced[client] = false;
}

public void L4D2FamilyShareCore_OnDetected(int client, int borrowerAccountId, int ownerAccountId)
{
	if (g_bManagerStopping || client <= 0 || client > MaxClients || !IsClientConnected(client))
		return;
	FamilyShareDetection detection;
	detection.coreGeneration = g_Runtime.coreGeneration;
	detection.clientSerial = GetClientSerial(client);
	detection.borrowerAccountId = borrowerAccountId;
	detection.ownerAccountId = ownerAccountId;
	detection.allowed = Policy_bIsAllowed(borrowerAccountId);
	detection.enforced = g_cvEnforce.BoolValue && g_bPolicyReady && !detection.allowed;
	detection.profileDeadline = GetEngineTime() + 30.0;
	GetClientName(client, detection.borrowerName, sizeof(detection.borrowerName));
	FamilyShare_AccountIdToSteamId64(borrowerAccountId, detection.borrowerSteamId64, sizeof(detection.borrowerSteamId64));
	FamilyShare_AccountIdToSteamId64(ownerAccountId, detection.ownerSteamId64, sizeof(detection.ownerSteamId64));
	FormatEx(detection.ownerProfileUrl, sizeof(detection.ownerProfileUrl), "https://steamcommunity.com/profiles/%s", detection.ownerSteamId64);
	FormatEx(detection.ownerName, sizeof(detection.ownerName), "%T", "UnknownOwner", LANG_SERVER);
	int ownerClient = FamilyShare_FindConnectedAccount(ownerAccountId);
	if (ownerClient != 0)
		GetClientName(ownerClient, detection.ownerName, sizeof(detection.ownerName));

	g_bClientEnforced[client] = detection.enforced;
	FamilyShare_LogDebug(g_Log, 1, "event=policy_evaluated borrower_accountid=%u allowed=%d enforced=%d ready=%d", borrowerAccountId, detection.allowed, detection.enforced, g_bPolicyReady);
	RefreshCoreCapabilities();
	int requestId = g_Runtime.canRequestProfile ? L4D2FamilyShareCore_RequestProfile(ownerAccountId) : 0;
	if (requestId > 0)
	{
		char key[48];
		FormatEx(key, sizeof(key), "%d:%d", detection.coreGeneration, requestId);
		g_smPendingDetections.SetArray(key, detection, sizeof(detection));
	}
	else
		ProcessDetection(detection);

	if (detection.enforced)
	{
		DataPack pack = new DataPack();
		pack.WriteCell(detection.clientSerial);
		pack.WriteCell(borrowerAccountId);
		pack.WriteCell(ownerAccountId);
		pack.WriteCell(detection.coreGeneration);
		RequestFrame(Frame_EnforceDetection, pack);
	}
}

public void Frame_EnforceDetection(any data)
{
	DataPack pack = view_as<DataPack>(data);
	pack.Reset();
	int client = GetClientFromSerial(pack.ReadCell());
	int borrowerAccountId = pack.ReadCell();
	int ownerAccountId = pack.ReadCell();
	int coreGeneration = pack.ReadCell();
	delete pack;
	RefreshCoreCapabilities();
	if (client == 0 || !IsClientConnected(client) || IsClientInKickQueue(client)
		|| !g_Runtime.canQueryCore || coreGeneration != g_Runtime.coreGeneration
		|| !g_cvEnforce.BoolValue || !g_bPolicyReady || Policy_bIsAllowed(borrowerAccountId)
		|| GetSteamAccountID(client, false) != borrowerAccountId
		|| L4D2FamilyShareCore_GetClientOwnerAccountId(client) != ownerAccountId)
		return;
	char message[192];
	FormatEx(message, sizeof(message), "%T", "KickClient", client);
	KickClient(client, "%s", message);
}

public void L4D2FamilyShareCore_OnProfileResolved(int requestId, int accountId,
	const char[] steamId64, const char[] personaName, const char[] profileUrl)
{
	char key[48];
	FormatEx(key, sizeof(key), "%d:%d", g_Runtime.coreGeneration, requestId);
	FamilyShareDetection detection;
	if (!g_smPendingDetections.GetArray(key, detection, sizeof(detection)) || detection.ownerAccountId != accountId)
		return;
	g_smPendingDetections.Remove(key);
	strcopy(detection.ownerSteamId64, sizeof(detection.ownerSteamId64), steamId64);
	strcopy(detection.ownerProfileUrl, sizeof(detection.ownerProfileUrl), profileUrl);
	if (personaName[0] != '\0')
		strcopy(detection.ownerName, sizeof(detection.ownerName), personaName);
	ProcessDetection(detection);
}

void ProcessDetection(FamilyShareDetection detection)
{
	FamilyShare_LogNormal(g_Log, "detection", "event=detected borrower_accountid=%u owner_accountid=%u allowed=%d enforced=%d borrower_sid64=%s owner_sid64=%s",
		detection.borrowerAccountId, detection.ownerAccountId, detection.allowed, detection.enforced,
		detection.borrowerSteamId64, detection.ownerSteamId64);
	Storage_Record(detection);
	if (g_cvAnnounce.BoolValue && !g_bManagerStopping)
	{
		CPrintToChatAll("%t %t", "Tag", "SharedCopyDetected", detection.borrowerName, detection.borrowerSteamId64);
		CPrintToChatAll("%t %t %t", "Tag", "OwnerProfileResolvedLabel", detection.ownerName, "OwnerProfileUrl", detection.ownerProfileUrl);
	}
}

public Action Timer_PendingDetections(Handle timer)
{
	FlushPendingDetections(false);
	return Plugin_Continue;
}

void FlushPendingDetections(bool all)
{
	StringMapSnapshot snapshot = g_smPendingDetections.Snapshot();
	char key[48];
	for (int i = 0; i < snapshot.Length; i++)
	{
		snapshot.GetKey(i, key, sizeof(key));
		FamilyShareDetection detection;
		if (!g_smPendingDetections.GetArray(key, detection, sizeof(detection)))
			continue;
		if (all || !g_Runtime.hasCore || detection.coreGeneration != g_Runtime.coreGeneration
			|| GetEngineTime() >= detection.profileDeadline)
		{
			g_smPendingDetections.Remove(key);
			ProcessDetection(detection);
		}
	}
	delete snapshot;
}

public void OnPluginEnd()
{
	g_bManagerStopping = true;
	FlushPendingDetections(true);
	Storage_Shutdown();
}

int Native_IsAccountAllowed(Handle plugin, int numParams)
{
	return Policy_bIsAllowed(GetNativeCell(1));
}

int Native_IsClientEnforced(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);
	return client > 0 && client <= MaxClients && IsClientConnected(client) && g_bClientEnforced[client];
}
