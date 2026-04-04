/*
	steamidtools_helpers.inc
	Copyright (C) 2026 AoC-Gamers
*/

#if defined _steamidtools_helpers_included
	#endinput
#endif
#define _steamidtools_helpers_included

#include <steamidtools_stock>

/**
 * Returns a command argument trimmed and without surrounding quotes.
 *
 * @param iArg          1-based argument index.
 * @param iArgs         Total command arguments.
 * @param szBuffer      Output buffer.
 * @param iMaxLength    Output buffer length.
 * @return              true when the argument exists, false otherwise.
 */
stock bool SteamIDTools_GetCmdArgNormalized(int iArg, int iArgs, char[] szBuffer, int iMaxLength)
{
	if (iArg < 1 || iArg > iArgs)
	{
		szBuffer[0] = '\0';
		return false;
	}

	GetCmdArg(iArg, szBuffer, iMaxLength);
	TrimString(szBuffer);
	StripQuotes(szBuffer);
	return true;
}

/**
 * Returns true if the string contains only decimal digits.
 */
stock bool SteamIDTools_IsNumericString(const char[] szValue)
{
	if (szValue[0] == '\0')
		return false;

	for (int i = 0; szValue[i] != '\0'; i++)
	{
		if (!IsCharNumeric(szValue[i]))
			return false;
	}

	return true;
}

/**
 * Joins normalized command arguments using a delimiter.
 */
stock void SteamIDTools_JoinCmdArgs(int iStartArg, int iArgs, char[] szBuffer, int iMaxLength, const char[] szDelimiter = " ")
{
	szBuffer[0] = '\0';

	char szPart[128];
	for (int i = iStartArg; i <= iArgs; i++)
	{
		if (!SteamIDTools_GetCmdArgNormalized(i, iArgs, szPart, sizeof(szPart)))
			continue;

		if (szPart[0] == '\0')
			continue;

		if (szBuffer[0] != '\0')
			StrCat(szBuffer, iMaxLength, szDelimiter);

		StrCat(szBuffer, iMaxLength, szPart);
	}
}

/**
 * Attempts to reconstruct an identity from one or more command arguments.
 *
 * This is useful for Source console input such as `STEAM_1:1:123`, which may
 * arrive split across multiple arguments when colons are not quoted.
 *
 * @param iStartArg         First argument index to inspect.
 * @param iArgs             Total command arguments.
 * @param szIdentity        Output buffer for the reconstructed identity.
 * @param iMaxLength        Output buffer length.
 * @param iNextArg          Receives the next argument index after the identity.
 * @param iMaxIdentityArgs  Maximum amount of arguments consumed while rebuilding.
 * @return                  true when a known identity format was reconstructed,
 *                          false when it falls back to the first normalized arg.
 */
stock bool SteamIDTools_TryGetIdentityFromCmdArgs(int iStartArg, int iArgs, char[] szIdentity, int iMaxLength, int &iNextArg, int iMaxIdentityArgs = 5)
{
	szIdentity[0] = '\0';
	iNextArg = iStartArg + 1;

	if (iStartArg < 1 || iStartArg > iArgs)
	{
		return false;
	}

	char szCandidate[128];
	char szPart[64];
	int iLastArg = iStartArg + iMaxIdentityArgs - 1;
	if (iLastArg > iArgs)
		iLastArg = iArgs;

	for (int i = iStartArg; i <= iLastArg; i++)
	{
		if (!SteamIDTools_GetCmdArgNormalized(i, iArgs, szPart, sizeof(szPart)))
			break;

		if (i == iStartArg)
			strcopy(szCandidate, sizeof(szCandidate), szPart);
		else
			StrCat(szCandidate, sizeof(szCandidate), szPart);

		SteamIDFormat eFormat = DetectSteamIDFormat(szCandidate);
		if (eFormat == STEAMID_FORMAT_UNKNOWN || eFormat == STEAMID_FORMAT_SPECIAL)
			continue;

		strcopy(szIdentity, iMaxLength, szCandidate);
		iNextArg = i + 1;
		return true;
	}

	SteamIDTools_GetCmdArgNormalized(iStartArg, iArgs, szIdentity, iMaxLength);
	iNextArg = iStartArg + 1;
	return false;
}
