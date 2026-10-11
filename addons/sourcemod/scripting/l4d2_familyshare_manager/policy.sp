StringMap g_smAllowedAccounts;
char g_sAllowedAccountsPath[PLATFORM_MAX_PATH];
bool g_bPolicyReady;

void Policy_Init()
{
	g_smAllowedAccounts = new StringMap();
	BuildPath(Path_SM, g_sAllowedAccountsPath, sizeof(g_sAllowedAccountsPath), "configs/l4d2_familyshare_allowed.kv");
	RegAdminCmd("sm_familyshare_allow", Command_AllowAccount, ADMFLAG_ROOT, "sm_familyshare_allow <borrower AccountID>");
	RegAdminCmd("sm_familyshare_disallow", Command_DisallowAccount, ADMFLAG_ROOT, "sm_familyshare_disallow <borrower AccountID>");
	RegAdminCmd("sm_familyshare_reload", Command_ReloadAllowedAccounts, ADMFLAG_ROOT, "Reload the borrower AccountID exceptions.");
	Policy_Reload();
}

bool Policy_Reload()
{
	KeyValues kv = new KeyValues("FamilyShareAllowed");
	if (!kv.ImportFromFile(g_sAllowedAccountsPath))
	{
		delete kv;
		LogError("[FamilyShare manager] event=allowlist_load_failed path=%s; keeping last valid policy, initial failure disables kicks", g_sAllowedAccountsPath);
		return false;
	}
	StringMap allowed = new StringMap();
	bool valid = true;
	if (kv.GotoFirstSubKey(false))
	{
		do
		{
			char key[32];
			kv.GetSectionName(key, sizeof(key));
			int accountId;
			char flag[16];
			kv.GetString("allowed", flag, sizeof(flag), "1");
			if (kv.GetDataType(NULL_STRING) != KvData_None || !FamilyShare_ParseAccountId(key, accountId)
				|| (!StrEqual(flag, "0") && !StrEqual(flag, "1")))
			{
				valid = false;
				break;
			}
			if (StrEqual(flag, "1"))
				allowed.SetValue(key, true);
		}
		while (kv.GotoNextKey(false));
	}
	delete kv;
	if (!valid)
	{
		delete allowed;
		LogError("[FamilyShare manager] event=allowlist_invalid_accountid; keeping last valid policy");
		return false;
	}
	delete g_smAllowedAccounts;
	g_smAllowedAccounts = allowed;
	g_bPolicyReady = true;
	FamilyShare_LogNormal(g_Log, "policy", "event=allowlist_loaded count=%d", allowed.Size);
	return true;
}

bool Policy_bIsAllowed(int borrowerAccountId)
{
	char key[16];
	FormatEx(key, sizeof(key), "%u", borrowerAccountId);
	int allowed;
	return borrowerAccountId != 0 && g_smAllowedAccounts.GetValue(key, allowed) && allowed != 0;
}

bool Policy_SetAllowed(int accountId, bool allow)
{
	if (!Policy_Reload())
		return false;
	KeyValues kv = new KeyValues("FamilyShareAllowed");
	if (!kv.ImportFromFile(g_sAllowedAccountsPath))
	{
		delete kv;
		return false;
	}
	char key[16];
	FormatEx(key, sizeof(key), "%u", accountId);
	if (allow)
	{
		kv.JumpToKey(key, true);
		kv.SetNum("allowed", 1);
		kv.Rewind();
	}
	else
		kv.DeleteKey(key);
	char temporary[PLATFORM_MAX_PATH];
	FormatEx(temporary, sizeof(temporary), "%s.tmp", g_sAllowedAccountsPath);
	bool saved = kv.ExportToFile(temporary) && RenameFile(g_sAllowedAccountsPath, temporary);
	delete kv;
	if (!saved || !Policy_Reload())
		return false;
	FamilyShare_LogNormal(g_Log, "policy", "event=allowlist_changed borrower_accountid=%u allowed=%d", accountId, allow);
	return true;
}

public Action Command_AllowAccount(int client, int args)
{
	return Policy_CommandSetAllowed(client, args, true);
}

public Action Command_DisallowAccount(int client, int args)
{
	return Policy_CommandSetAllowed(client, args, false);
}

Action Policy_CommandSetAllowed(int client, int args, bool allowed)
{
	char input[32];
	GetCmdArg(1, input, sizeof(input));
	int accountId;
	if (args != 1 || !FamilyShare_ParseAccountId(input, accountId))
	{
		ReplyToCommand(client, "%T", "AllowAccountUsage", client);
		return Plugin_Handled;
	}
	if (!Policy_SetAllowed(accountId, allowed))
		ReplyToCommand(client, "%T", "AllowAccountSaveFailed", client);
	else
		ReplyToCommand(client, "%T", "AllowAccountUpdated", client, input, allowed ? 1 : 0);
	return Plugin_Handled;
}

public Action Command_ReloadAllowedAccounts(int client, int args)
{
	if (Policy_Reload())
		ReplyToCommand(client, "%T", "AllowAccountsReloaded", client, g_smAllowedAccounts.Size);
	else
		ReplyToCommand(client, "%T", "AllowAccountSaveFailed", client);
	return Plugin_Handled;
}
