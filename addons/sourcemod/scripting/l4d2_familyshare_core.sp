#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <steamworks>
#include <l4d2_familyshare_stock>

#undef REQUIRE_PLUGIN
#include <steamidtools>
#define REQUIRE_PLUGIN

enum struct FamilyShareCoreRuntimeState
{
	bool lateLoad;
	bool hasSteamIDTools;
	bool canConvert;
	int providerGeneration;

	void Reset()
	{
		this.lateLoad = false;
		this.hasSteamIDTools = false;
		this.canConvert = false;
		this.providerGeneration = 0;
	}

	void DetectLibraries()
	{
		this.hasSteamIDTools = LibraryExists(STEAMIDTOOLS_LIBRARY);
		if (!this.hasSteamIDTools)
			this.canConvert = false;
	}

	void SetLibraryAvailability(const char[] name, bool available)
	{
		if (!StrEqual(name, STEAMIDTOOLS_LIBRARY))
			return;
		this.hasSteamIDTools = available;
		if (!available)
		{
			this.canConvert = false;
			this.providerGeneration++;
		}
	}
}

FamilyShareCoreRuntimeState g_Runtime;
FamilyShareRuntimeLog g_Log;
bool g_bDetected[MAXPLAYERS + 1];
int g_iBorrowerAccountId[MAXPLAYERS + 1];
int g_iOwnerAccountId[MAXPLAYERS + 1];
GlobalForward g_fwdDetected;
GlobalForward g_fwdProfileResolved;

#include "l4d2_familyshare_core/profiles.sp"

public Plugin myinfo =
{
	name = "L4D2 Family Share Core",
	author = "Lechuga",
	description = "Family Sharing detection, identities and optional profile enrichment.",
	version = "2.0.0",
	url = "https://github.com/AoC-Gamers/L4D2-Family-Share"
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int errMax)
{
	g_Runtime.Reset();
	g_Runtime.lateLoad = late;
	RegPluginLibrary("l4d2_familyshare_core");
	CreateNative("L4D2FamilyShareCore_IsClientDetected", Native_IsClientDetected);
	CreateNative("L4D2FamilyShareCore_GetClientBorrowerAccountId", Native_GetClientBorrowerAccountId);
	CreateNative("L4D2FamilyShareCore_GetClientOwnerAccountId", Native_GetClientOwnerAccountId);
	CreateNative("L4D2FamilyShareCore_RequestProfile", Native_RequestProfile);
	g_fwdDetected = new GlobalForward("L4D2FamilyShareCore_OnDetected", ET_Ignore, Param_Cell, Param_Cell, Param_Cell);
	g_fwdProfileResolved = new GlobalForward("L4D2FamilyShareCore_OnProfileResolved", ET_Ignore, Param_Cell, Param_Cell, Param_String, Param_String, Param_String);
	return APLRes_Success;
}

public void OnPluginStart()
{
	g_Log.Init("core", "0");
	Profiles_Init();
	AutoExecConfig(true, "l4d2_familyshare_core");
}

public void OnAllPluginsLoaded()
{
	g_Runtime.DetectLibraries();
}

public void OnConfigsExecuted()
{
	FamilyShare_LogNormal(g_Log, "lifecycle", "event=configured late_load=%d steamidtools=%d", g_Runtime.lateLoad, g_Runtime.hasSteamIDTools);
}

public void OnLibraryAdded(const char[] name)
{
	g_Runtime.SetLibraryAvailability(name, true);
}

public void OnLibraryRemoved(const char[] name)
{
	g_Runtime.SetLibraryAvailability(name, false);
}

public void OnPluginEnd()
{
	Profiles_Shutdown();
}

public void OnClientConnected(int client)
{
	ResetClientState(client);
}

public void OnClientDisconnect(int client)
{
	ResetClientState(client);
}

void ResetClientState(int client)
{
	g_bDetected[client] = false;
	g_iBorrowerAccountId[client] = 0;
	g_iOwnerAccountId[client] = 0;
}

bool bIsHumanConnected(int client)
{
	return client > 0 && client <= MaxClients && IsClientConnected(client) && !IsFakeClient(client);
}

public void SteamWorks_OnValidateClient(int ownerauthid, int authid)
{
	int client = FamilyShare_FindConnectedAccount(authid);
	if (client == 0 || ownerauthid == 0)
		return;
	if (ownerauthid == authid)
	{
		ResetClientState(client);
		return;
	}
	if (g_bDetected[client] && g_iBorrowerAccountId[client] == authid && g_iOwnerAccountId[client] == ownerauthid)
		return;

	g_bDetected[client] = true;
	g_iBorrowerAccountId[client] = authid;
	g_iOwnerAccountId[client] = ownerauthid;
	FamilyShare_LogDebug(g_Log, 1, "event=detected borrower_accountid=%u owner_accountid=%u", authid, ownerauthid);
	Call_StartForward(g_fwdDetected);
	Call_PushCell(client);
	Call_PushCell(authid);
	Call_PushCell(ownerauthid);
	Call_Finish();
}

int Native_IsClientDetected(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);
	return bIsHumanConnected(client) && g_bDetected[client];
}

int Native_GetClientBorrowerAccountId(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);
	return bIsHumanConnected(client) && g_bDetected[client] ? g_iBorrowerAccountId[client] : 0;
}

int Native_GetClientOwnerAccountId(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);
	return bIsHumanConnected(client) && g_bDetected[client] ? g_iOwnerAccountId[client] : 0;
}
