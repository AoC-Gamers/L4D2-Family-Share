#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <l4d2_familyshare>
#include <bansystem_access>

ConVar g_cvEnabled;
ConVar g_cvBanReason;
ConVar g_cvDebug;
StringMap g_smPendingChecks;

public Plugin myinfo =
{
	name = "L4D2 Family Share Ban Bridge",
	author = "Lechuga",
	description = "Bridge for applying ban sanctions from family share owner checks.",
	version = "1.4.0",
	url = "https://github.com/AoC-Gamers/L4D2-Family-Share"
};

public void OnPluginStart()
{
	g_cvEnabled = CreateConVar("l4d2_familyshare_ban_enabled", "1", "Enable FamilyShare to BanSystem access mirroring.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvBanReason = CreateConVar("l4d2_familyshare_ban_reason", "Family sharing from access-banned owner", "Reason used when mirroring access bans from owner to borrower.", FCVAR_NONE);
	g_cvDebug = CreateConVar("l4d2_familyshare_ban_debug", "0", "Enable debug logging for FamilyShare access mirroring.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_smPendingChecks = new StringMap();

	AutoExecConfig(true, "l4d2_familyshare_ban");

}

public void OnPluginEnd()
{
	ClearPendingChecks();

	if (g_smPendingChecks != null)
		delete g_smPendingChecks;
}

public void L4D2FamilyShare_OnDetected(int client, int borrowerAccountId, int ownerAccountId, bool enforced)
{
	if (!g_cvEnabled.BoolValue)
		return;

	if (borrowerAccountId <= 0 || ownerAccountId <= 0)
		return;

	if (!BSAccess_IsDatabaseReady())
	{
		DebugLog("skip borrower=%d owner=%d database_not_ready=1", borrowerAccountId, ownerAccountId);
		return;
	}

	int requestId = BSAccess_RequestBanInfoByAccountId(ownerAccountId);
	if (requestId <= 0)
	{
		DebugLog("skip borrower=%d owner=%d request_failed=1", borrowerAccountId, ownerAccountId);
		return;
	}

	DataPack pack = new DataPack();
	pack.WriteCell(borrowerAccountId);
	pack.WriteCell(ownerAccountId);
	pack.WriteCell(client);
	pack.WriteCell(enforced ? 1 : 0);

	char key[16];
	IntToString(requestId, key, sizeof(key));
	g_smPendingChecks.SetValue(key, pack);
	DebugLog("queued borrower=%d owner=%d request=%d client=%d enforced=%d", borrowerAccountId, ownerAccountId, requestId, client, enforced ? 1 : 0);
}

public void BSAccess_OnAccountIdBanInfo(int requestId, int accountId, bool success, bool banned, int banLength)
{
	if (g_smPendingChecks == null)
		return;

	char key[16];
	IntToString(requestId, key, sizeof(key));

	int packValue = 0;
	if (!g_smPendingChecks.GetValue(key, packValue))
		return;

	g_smPendingChecks.Remove(key);

	DataPack pack = view_as<DataPack>(packValue);
	if (pack == null)
		return;

	pack.Reset();
	int borrowerAccountId = pack.ReadCell();
	int ownerAccountId = pack.ReadCell();
	int borrowerClient = pack.ReadCell();
	bool enforced = pack.ReadCell() == 1;
	delete pack;

	DebugLog("resolved borrower=%d owner=%d request=%d success=%d banned=%d length=%d client=%d enforced=%d", borrowerAccountId, ownerAccountId, requestId, success ? 1 : 0, banned ? 1 : 0, banLength, borrowerClient, enforced ? 1 : 0);

	if (!g_cvEnabled.BoolValue || !success || !banned || borrowerAccountId <= 0)
		return;

	char reason[192];
	char context[192];
	g_cvBanReason.GetString(reason, sizeof(reason));
	Format(context, sizeof(context), "familyshare_owner_accountid=%d owner_ban_length=%d", ownerAccountId, banLength);

	bool accepted = BSAccess_AddBanByAccountId(0, borrowerAccountId, banLength, reason, context);
	DebugLog("ban borrower=%d owner=%d length=%d accepted=%d", borrowerAccountId, ownerAccountId, banLength, accepted ? 1 : 0);
}

void ClearPendingChecks()
{
	if (g_smPendingChecks == null)
		return;

	StringMapSnapshot snapshot = g_smPendingChecks.Snapshot();
	if (snapshot == null)
		return;

	char key[16];
	for (int i = 0; i < snapshot.Length; i++)
	{
		snapshot.GetKey(i, key, sizeof(key));

		int packValue = 0;
		if (!g_smPendingChecks.GetValue(key, packValue))
			continue;

		DataPack pack = view_as<DataPack>(packValue);
		if (pack != null)
			delete pack;
	}

	delete snapshot;
	g_smPendingChecks.Clear();
}

void DebugLog(const char[] format, any ...)
{
	if (g_cvDebug == null || !g_cvDebug.BoolValue)
		return;

	char buffer[256];
	VFormat(buffer, sizeof(buffer), format, 2);
	LogMessage("[FamilyShareBSAccess] %s", buffer);
}