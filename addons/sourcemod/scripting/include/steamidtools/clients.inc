/** SteamIDTools adapters for connected SourceMod clients. */
#if defined _steamidtools_clients_included
    #endinput
#endif
#define _steamidtools_clients_included
#include <steamidtools>
#include <clients>

/**
 * Gets the AccountID of a connected client
 *
 * @param iClient       Client index
 * @return              AccountID, or 0 if invalid
 */
stock int GetClientAccountID(int iClient)
{
	if (!IsValidClientIndex(iClient) || !IsClientInGame(iClient))
	{
		return 0;
	}

	return GetSteamAccountID(iClient);
}

/**
 * Finds the client index by their Steam Account ID.
 *
 * @param iAccountId	Steam Account ID to search for.
 * @return				Client index (1-based) if found,
 *                      0 if the provided Account ID is invalid,
 *                      or -1 if no matching client is found.
 */
stock int FindClientByAccountID(int iAccountId)
{
	if (iAccountId == 0)
	{
		return 0;
	}

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClientIndex(i) || !IsClientInGame(i))
		{
			continue;
		}

		if (GetSteamAccountID(i) != iAccountId)
		{
			continue;
		}

		return i;
	}

	return -1;
}

/**
 * Finds a client by SteamID2
 *
 * @param szSteamId2    SteamID2 to search for
 * @return              Client index, or 0 if not found
 */
stock int FindClientBySteamID2(const char[] szSteamId2)
{
	int iAccountId = SteamID2ToAccountID(szSteamId2);
	return FindClientByAccountID(iAccountId);
}

/**
 * Finds a client by SteamID3
 *
 * @param szSteamId3    SteamID3 to search for
 * @return              Client index, or 0 if not found
 */
stock int FindClientBySteamID3(const char[] szSteamId3)
{
	int iAccountId = SteamID3ToAccountID(szSteamId3);
	return FindClientByAccountID(iAccountId);
}

/**
 * Validates if a client index is valid (internal helper)
 *
 * @param iClient       Client index
 * @return              true if valid, false otherwise
 */
stock bool IsValidClientIndex(int iClient)
{
	return (iClient > 0 && iClient <= MaxClients);
}

