/**
 * @file l4d2_familyshare_stock.inc
 * @brief Shared identity adapters, allowlist parsing and per-plugin logging.
 *
 * These helpers compile into the consumer and register no runtime library.
 * SteamIDTools stocks provide decimal conversion without requiring the
 * SteamIDTools plugin to be loaded. Each consumer owns its logger instance.
 *
 * AccountIDs use unsigned 32-bit cell bit patterns. Format them with %u;
 * negative signed values may represent valid AccountIDs. Zero is invalid.
 */

#if defined _l4d2_familyshare_stock_included_
	#endinput
#endif
#define _l4d2_familyshare_stock_included_

#include <sourcemod>
#include <steamidtools_stock>

/**
 * Parses the canonical decimal AccountID used by the external allowlist.
 *
 * Accepts values from 1 through 4294967295. Signs, whitespace, leading zeroes
 * and nondecimal characters are rejected. The canonical spelling check is
 * applied before delegating unsigned conversion to SteamIDTools.
 *
 * @param input         Decimal AccountID string to parse.
 * @param accountId     Receives the unsigned cell bit pattern on success.
 *                      Only use this value when the function returns true.
 * @return              True if input is a canonical nonzero AccountID,
 *                      false otherwise.
 */
stock bool FamilyShare_ParseAccountId(const char[] input, int &accountId)
{
	int length = strlen(input);
	if (length == 0 || length > 10 || (length > 1 && input[0] == '0'))
		return false;
	return SteamIDTools_TryParseAccountID(input, accountId);
}

/**
 * Converts an AccountID to the SteamID64 of an individual/public account.
 *
 * Uses SteamIDTools offline conversion for the complete uint32 AccountID
 * range, including values stored in negative signed cells.
 *
 * @param accountId     Nonzero AccountID stored as an unsigned cell bit pattern.
 * @param output        Destination buffer. Cleared on failure if maxlen > 0.
 * @param maxlen        Buffer capacity in bytes, including the null terminator.
 *                      At least STEAMID64_EXACT_LENGTH + 1 bytes are required.
 * @return              True on success, false for zero, insufficient capacity,
 *                      or decimal conversion failure.
 */
stock bool FamilyShare_AccountIdToSteamId64(int accountId, char[] output, int maxlen)
{
	if (maxlen <= 0)
		return false;
	output[0] = '\0';
	if (accountId == 0 || maxlen < STEAMID64_EXACT_LENGTH + 1)
		return false;
	return SteamIDTools_AccountIDToSteamID64(accountId, output, maxlen);
}

/**
 * Finds a connected human client by AccountID, including before entering the
 * game. Uses the license identity without requiring completed authentication.
 *
 * SteamIDTools FindClientByAccountID requires an in-game client. This helper
 * also covers connected clients before entering the game and returns one
 * consistent sentinel for invalid or missing accounts.
 *
 * @param accountId     AccountID to find, stored as an unsigned cell bit pattern.
 * @return              Matching client index, or 0 for zero or no connected
 *                      human match. Save a client serial for deferred actions.
 */
stock int FamilyShare_FindConnectedAccount(int accountId)
{
	if (accountId == 0)
		return 0;
	for (int client = 1; client <= MaxClients; client++)
	{
		if (IsClientConnected(client) && !IsFakeClient(client) && GetSteamAccountID(client, false) == accountId)
			return client;
	}
	return 0;
}

/**
 * Logger state owned by one consuming plugin.
 *
 * Declare one instance in each plugin, initialize it once in OnPluginStart,
 * and pass it to the logging stocks. Files and ConVars are independent for each module.
 */
enum struct FamilyShareRuntimeLog
{
	/** Log mode: 0 = off, 1 = normal, 2 = normal and selected debug categories. */
	ConVar mode;
	/** Debug mask: identity/policy = 1, async/SQL = 2, lifecycle = 4, all = 7. */
	ConVar mask;
	/** Absolute log file path resolved from the SourceMod directory. */
	char path[PLATFORM_MAX_PATH];
	/** True after the destination directory and append access are verified. */
	bool ready;

	/**
	 * Creates this module's ConVars and prepares logs/familyshare/<module>.log.
	 * The append probe is closed immediately; no file handle remains open.
	 * A setup failure leaves logging disabled and emits a SourceMod error.
	 *
	 * @param module        Trusted module name, such as "core" or "manager".
	 * @param defaultMode   Default mode as a string: "0", "1", or "2".
	 * @noreturn
	 */
	void Init(const char[] module, const char[] defaultMode)
	{
		char name[96];
		FormatEx(name, sizeof(name), "l4d2_familyshare_%s_log_mode", module);
		this.mode = CreateConVar(name, defaultMode, "Own log: 0=off, 1=normal, 2=debug.", FCVAR_NONE, true, 0.0, true, 2.0);
		FormatEx(name, sizeof(name), "l4d2_familyshare_%s_debug_mask", module);
		this.mask = CreateConVar(name, "0", "Debug categories: identity/policy=1, async/SQL=2, lifecycle=4, all=7.", FCVAR_NONE, true, 0.0, true, 7.0);
		char directory[PLATFORM_MAX_PATH];
		BuildPath(Path_SM, directory, sizeof(directory), "logs/familyshare");
		if (!DirExists(directory) && !CreateDirectory(directory, 511))
		{
			LogError("[FamilyShare %s] event=log_init_failed stage=directory", module);
			return;
		}
		BuildPath(Path_SM, this.path, sizeof(this.path), "logs/familyshare/%s.log", module);
		File file = OpenFile(this.path, "a");
		if (file == null)
		{
			LogError("[FamilyShare %s] event=log_init_failed stage=file", module);
			return;
		}
		delete file;
		this.ready = true;
	}

	/**
	 * Checks whether normal messages can be written.
	 *
	 * @return              True if initialization succeeded and mode is 1 or 2.
	 */
	bool bNormalEnabled()
	{
		return this.ready && this.mode.IntValue != 0;
	}

	/**
	 * Checks whether a debug category is enabled.
	 *
	 * @param category      Category bit or combination of bits to check.
	 * @return              True if initialized, mode is 2 and at least one
	 *                      requested category bit is enabled in the mask.
	 */
	bool bDebugEnabled(int category)
	{
		return this.ready && this.mode.IntValue == 2 && (this.mask.IntValue & category) != 0;
	}
}

/**
 * Writes a normal message prefixed with [category] to this plugin's log file.
 * Checks readiness and mode before formatting. The formatted message buffer
 * holds at most 1023 bytes plus the null terminator.
 *
 * @param logger        Consumer-owned, initialized logger instance.
 * @param category      Text category to prefix to the message.
 * @param message       printf-style format string.
 * @param ...           Format arguments.
 * @noreturn
 */
stock void FamilyShare_LogNormal(FamilyShareRuntimeLog logger, const char[] category, const char[] message, any ...)
{
	if (!logger.bNormalEnabled())
		return;
	char text[1024];
	VFormat(text, sizeof(text), message, 4);
	LogToFileEx(logger.path, "[%s] %s", category, text);
}

/**
 * Writes a debug message prefixed with [debug] to this plugin's log file.
 * Checks readiness, mode and category mask before formatting. The formatted
 * message buffer holds at most 1023 bytes plus the null terminator.
 *
 * @param logger        Consumer-owned, initialized logger instance.
 * @param category      Debug category bit or combination of bits to check.
 * @param message       printf-style format string.
 * @param ...           Format arguments.
 * @noreturn
 */
stock void FamilyShare_LogDebug(FamilyShareRuntimeLog logger, int category, const char[] message, any ...)
{
	if (!logger.bDebugEnabled(category))
		return;
	char text[1024];
	VFormat(text, sizeof(text), message, 4);
	LogToFileEx(logger.path, "[debug] %s", text);
}
