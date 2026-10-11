#define FAMILYSHARE_SQL_PENDING_KV "data/l4d2_familyshare_pending.kv"

ConVar g_cvSqlEnable;
ConVar g_cvSqlConfig;
ConVar g_cvSqlTable;
ConVar g_cvDebugSql;
Database g_dbHistory;
ArrayList g_aPendingHistory;
StringMap g_smHistoryInFlight;
Handle g_hHistoryRetry;
int g_iStorageGeneration;
int g_iStorageAttempts;
int g_iNextHistoryId = 1;
bool g_bStorageReady;
bool g_bPendingFileHealthy = true;
char g_sPendingHistoryPath[PLATFORM_MAX_PATH];
char g_sHistoryTable[64];

void Storage_Init()
{
	g_cvSqlEnable = CreateConVar("l4d2_familyshare_sql", "0", "Enable optional SQL detection history in manager.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvSqlConfig = CreateConVar("l4d2_familyshare_sql_config", "default", "databases.cfg entry for detection history.");
	g_cvSqlTable = CreateConVar("l4d2_familyshare_sql_table", "l4d2_familyshare", "Detection history table (letters, digits and underscores only).");
	g_cvDebugSql = CreateConVar("l4d2_familyshare_sql_debug", "0", "Enable SQL diagnostics in the manager log when its mode is not off.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_cvSqlEnable.AddChangeHook(OnHistorySettingsChanged);
	g_cvSqlConfig.AddChangeHook(OnHistorySettingsChanged);
	g_cvSqlTable.AddChangeHook(OnHistorySettingsChanged);
	g_aPendingHistory = new ArrayList(sizeof(FamilyShareDetection));
	g_smHistoryInFlight = new StringMap();
	BuildPath(Path_SM, g_sPendingHistoryPath, sizeof(g_sPendingHistoryPath), FAMILYSHARE_SQL_PENDING_KV);
	Storage_LoadPending();
}

void Storage_Debug(const char[] message, any ...)
{
	if (!g_Log.bDebugEnabled(2) && !(g_cvDebugSql.BoolValue && g_Log.bNormalEnabled()))
		return;
	char text[256];
	VFormat(text, sizeof(text), message, 2);
	LogToFileEx(g_Log.path, "[sql] %s", text);
}

public void OnHistorySettingsChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	Storage_Reconcile();
}

void Storage_Reconcile()
{
	g_iStorageGeneration++;
	g_iStorageAttempts = 0;
	g_bStorageReady = false;
	delete g_hHistoryRetry;
	delete g_dbHistory;
	if (!g_cvSqlEnable.BoolValue || g_bManagerStopping)
		return;
	char configuredTable[256];
	g_cvSqlTable.GetString(configuredTable, sizeof(configuredTable));
	if (strlen(configuredTable) >= sizeof(g_sHistoryTable))
	{
		LogError("[FamilyShare manager] event=history_invalid_table_length");
		return;
	}
	strcopy(g_sHistoryTable, sizeof(g_sHistoryTable), configuredTable);
	int length = strlen(g_sHistoryTable);
	if (length == 0)
		return;
	for (int i = 0; i < length; i++)
	{
		if (!IsCharAlpha(g_sHistoryTable[i]) && !IsCharNumeric(g_sHistoryTable[i]) && g_sHistoryTable[i] != '_')
		{
			LogError("[FamilyShare manager] event=history_invalid_table");
			return;
		}
	}
	Storage_Connect();
}

void Storage_Connect()
{
	char config[64];
	g_cvSqlConfig.GetString(config, sizeof(config));
	if (!config[0] || !g_cvSqlEnable.BoolValue || g_bManagerStopping)
		return;
	g_iStorageAttempts++;
	Storage_Debug("event=connecting generation=%d attempt=%d", g_iStorageGeneration, g_iStorageAttempts);
	Database.Connect(OnHistoryConnected, config, g_iStorageGeneration);
}

public void OnHistoryConnected(Database db, const char[] error, any generation)
{
	if (generation != g_iStorageGeneration || !g_cvSqlEnable.BoolValue || g_bManagerStopping)
	{
		delete db;
		return;
	}
	if (db == null)
	{
		LogError("[FamilyShare manager] event=history_connection_failed error=%s", error);
		if (g_iStorageAttempts < 2)
			g_hHistoryRetry = CreateTimer(30.0, Timer_RetryHistory, generation);
		return;
	}
	g_dbHistory = db;
	char driver[16];
	db.Driver.GetIdentifier(driver, sizeof(driver));
	char query[2048];
	if (StrEqual(driver, "sqlite"))
	{
		FormatEx(query, sizeof(query), "CREATE TABLE IF NOT EXISTS `%s` (id INTEGER PRIMARY KEY AUTOINCREMENT, borrower_name TEXT NOT NULL, borrower_accountid INTEGER NOT NULL, borrower_steamid64 TEXT, owner_name TEXT NOT NULL, owner_accountid INTEGER NOT NULL DEFAULT 0, owner_steamid64 TEXT, allowed INTEGER NOT NULL DEFAULT 0, enforced INTEGER NOT NULL DEFAULT 0, created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP);", g_sHistoryTable);
		db.Query(OnHistoryTableCreated, query, generation);
	}
	else
		Storage_CheckSchema(generation);
}

public void OnHistoryTableCreated(Database db, DBResultSet results, const char[] error, any generation)
{
	if (generation != g_iStorageGeneration || db != g_dbHistory)
		return;
	if (error[0])
	{
		LogError("[FamilyShare manager] event=history_table_failed error=%s", error);
		return;
	}
	Storage_CheckSchema(generation);
}

void Storage_CheckSchema(int generation)
{
	char query[512];
	FormatEx(query, sizeof(query), "SELECT borrower_name,borrower_accountid,borrower_steamid64,owner_name,owner_accountid,owner_steamid64,allowed,enforced FROM `%s` LIMIT 0;", g_sHistoryTable);
	g_dbHistory.Query(OnHistorySchemaChecked, query, generation);
}

public void OnHistorySchemaChecked(Database db, DBResultSet results, const char[] error, any generation)
{
	if (generation != g_iStorageGeneration || db != g_dbHistory)
		return;
	if (error[0])
	{
		LogError("[FamilyShare manager] event=history_schema_failed apply the documented schema migration; error=%s", error);
		return;
	}
	g_bStorageReady = true;
	FamilyShare_LogNormal(g_Log, "sql", "event=connected pending=%d", g_aPendingHistory.Length);
	Storage_Flush();
}

public Action Timer_RetryHistory(Handle timer, any generation)
{
	g_hHistoryRetry = null;
	if (generation == g_iStorageGeneration)
		Storage_Connect();
	return Plugin_Stop;
}

void Storage_Record(FamilyShareDetection detection)
{
	if (!g_cvSqlEnable.BoolValue)
		return;
	detection.historyId = g_iNextHistoryId++;
	g_aPendingHistory.PushArray(detection, sizeof(detection));
	Storage_SavePending();
	Storage_Flush();
}

void Storage_Flush()
{
	if (!g_bStorageReady || g_dbHistory == null || !g_cvSqlEnable.BoolValue || g_bManagerStopping)
		return;
	for (int i = 0; i < g_aPendingHistory.Length; i++)
	{
		FamilyShareDetection detection;
		g_aPendingHistory.GetArray(i, detection, sizeof(detection));
		char key[16];
		IntToString(detection.historyId, key, sizeof(key));
		int generation;
		if (g_smHistoryInFlight.GetValue(key, generation))
			continue;
		char borrowerName[2 * MAX_NAME_LENGTH + 1];
		char ownerName[2 * MAX_NAME_LENGTH + 1];
		char borrowerSid[65];
		char ownerSid[65];
		if (!g_dbHistory.Escape(detection.borrowerName, borrowerName, sizeof(borrowerName))
			|| !g_dbHistory.Escape(detection.ownerName, ownerName, sizeof(ownerName))
			|| !g_dbHistory.Escape(detection.borrowerSteamId64, borrowerSid, sizeof(borrowerSid))
			|| !g_dbHistory.Escape(detection.ownerSteamId64, ownerSid, sizeof(ownerSid)))
			continue;
		char query[2048];
		FormatEx(query, sizeof(query), "INSERT INTO `%s` (borrower_name,borrower_accountid,borrower_steamid64,owner_name,owner_accountid,owner_steamid64,allowed,enforced) VALUES ('%s',%u,NULLIF('%s',''),'%s',%u,NULLIF('%s',''),%d,%d);",
			g_sHistoryTable, borrowerName, detection.borrowerAccountId, borrowerSid,
			ownerName, detection.ownerAccountId, ownerSid, detection.allowed, detection.enforced);
		DataPack pack = new DataPack();
		pack.WriteCell(detection.historyId);
		pack.WriteCell(g_iStorageGeneration);
		g_smHistoryInFlight.SetValue(key, g_iStorageGeneration);
		g_dbHistory.Query(OnHistoryInserted, query, pack);
		Storage_Debug("event=insert_queued record=%d borrower_accountid=%u", detection.historyId, detection.borrowerAccountId);
	}
}

public void OnHistoryInserted(Database db, DBResultSet results, const char[] error, any data)
{
	DataPack pack = view_as<DataPack>(data);
	pack.Reset();
	int historyId = pack.ReadCell();
	int generation = pack.ReadCell();
	delete pack;
	char key[16];
	IntToString(historyId, key, sizeof(key));
	int activeGeneration;
	if (!g_smHistoryInFlight.GetValue(key, activeGeneration) || activeGeneration != generation)
		return;
	g_smHistoryInFlight.Remove(key);
	if (error[0])
	{
		LogError("[FamilyShare manager] event=history_insert_failed record=%d error=%s", historyId, error);
		// Preserve the persisted event; retry on the next reconciliation.
		if (generation == g_iStorageGeneration)
			g_bStorageReady = false;
		return;
	}
	for (int i = 0; i < g_aPendingHistory.Length; i++)
	{
		FamilyShareDetection detection;
		g_aPendingHistory.GetArray(i, detection, sizeof(detection));
		if (detection.historyId == historyId)
		{
			g_aPendingHistory.Erase(i);
			break;
		}
	}
	Storage_SavePending();
}

void Storage_LoadPending()
{
	if (!FileExists(g_sPendingHistoryPath))
		return;
	KeyValues kv = new KeyValues("FamilySharePending");
	if (!kv.ImportFromFile(g_sPendingHistoryPath))
	{
		delete kv;
		g_bPendingFileHealthy = false;
		LogError("[FamilyShare manager] event=pending_import_failed; original file preserved");
		return;
	}
	if (kv.JumpToKey("events") && kv.GotoFirstSubKey(false))
	{
		do
		{
			FamilyShareDetection detection;
			detection.historyId = g_iNextHistoryId++;
			detection.borrowerAccountId = kv.GetNum("borrower_accountid");
			detection.ownerAccountId = kv.GetNum("owner_accountid");
			kv.GetString("borrower_name", detection.borrowerName, sizeof(detection.borrowerName));
			kv.GetString("borrower_steamid64", detection.borrowerSteamId64, sizeof(detection.borrowerSteamId64));
			kv.GetString("owner_name", detection.ownerName, sizeof(detection.ownerName), "Unknown Owner");
			kv.GetString("owner_steamid64", detection.ownerSteamId64, sizeof(detection.ownerSteamId64));
			detection.allowed = kv.GetNum("allowed", 0) == 1;
			detection.enforced = kv.GetNum("enforced", 0) == 1;
			if (kv.GetDataType(NULL_STRING) != KvData_None || detection.borrowerAccountId == 0)
			{
				g_bPendingFileHealthy = false;
				LogError("[FamilyShare manager] event=pending_invalid_record; original file preserved");
			}
			else
				g_aPendingHistory.PushArray(detection, sizeof(detection));
		}
		while (kv.GotoNextKey(false));
	}
	delete kv;
}

void Storage_SavePending()
{
	if (!g_bPendingFileHealthy)
		return;
	KeyValues kv = new KeyValues("FamilySharePending");
	kv.JumpToKey("events", true);
	for (int i = 0; i < g_aPendingHistory.Length; i++)
	{
		FamilyShareDetection detection;
		g_aPendingHistory.GetArray(i, detection, sizeof(detection));
		char key[32];
		FormatEx(key, sizeof(key), "event_%d", detection.historyId);
		kv.JumpToKey(key, true);
		kv.SetNum("borrower_accountid", detection.borrowerAccountId);
		kv.SetNum("owner_accountid", detection.ownerAccountId);
		kv.SetString("borrower_name", detection.borrowerName);
		kv.SetString("borrower_steamid64", detection.borrowerSteamId64);
		kv.SetString("owner_name", detection.ownerName);
		kv.SetString("owner_steamid64", detection.ownerSteamId64);
		kv.SetNum("allowed", detection.allowed ? 1 : 0);
		kv.SetNum("enforced", detection.enforced ? 1 : 0);
		kv.GoBack();
	}
	kv.Rewind();
	char temporary[PLATFORM_MAX_PATH];
	FormatEx(temporary, sizeof(temporary), "%s.tmp", g_sPendingHistoryPath);
	if (!kv.ExportToFile(temporary) || !RenameFile(g_sPendingHistoryPath, temporary))
		LogError("[FamilyShare manager] event=pending_save_failed");
	delete kv;
}

void Storage_Shutdown()
{
	Storage_SavePending();
	delete g_hHistoryRetry;
	// In-flight rows remain in the outbox until their INSERT is acknowledged.
}
