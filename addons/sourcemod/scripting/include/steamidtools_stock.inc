/*
	steamidtools_stock.inc
	Copyright (C) 2025 AoC-Gamers
*/

#if defined _steamidtools_stock_included
	#endinput
#endif
#define _steamidtools_stock_included

#include <sourcemod>
#include <clients>

#define STEAMIDTOOLS_VERSION "2.5.0"

#define ACCOUNTID_LENGTH	   16
#define STEAMID64_EXACT_LENGTH 17
#define STEAMID2_MIN_LENGTH	   11
#define STEAMID3_MIN_LENGTH	   7
#define STEAMID64_BASE		   76561197960265728
#define STEAMID64_BASE_STRING  "76561197960265728"
#define STEAMID64_MIN_INDIVIDUAL_STRING "76561197960265729"
#define STEAMID64_MAX_INDIVIDUAL_STRING "76561202255233023"
#define ACCOUNTID_UINT32_MAX_STRING "4294967295"

// AccountIDs are uint32 bit patterns, even when the SourcePawn cell is negative.
// Zero is the invalid identity sentinel. SteamID64 values remain decimal strings.

#define URL_VANITY "https://steamcommunity.com/id/"
#define URL_SID    "https://steamcommunity.com/profiles/"

#if !defined STEAMIDTOOLS_SID2_UNIVERSE
	#define STEAMIDTOOLS_SID2_UNIVERSE 1
#endif

/**
 * Enum for different SteamID formats
 */
enum SteamIDFormat
{
	STEAMID_FORMAT_UNKNOWN = 0,
	STEAMID_FORMAT_STEAMID2,
	STEAMID_FORMAT_STEAMID3,
	STEAMID_FORMAT_STEAMID64,
	STEAMID_FORMAT_ACCOUNTID,
	STEAMID_FORMAT_SPECIAL
}

/**
 * Validates if a SteamID2 has valid format
 * Expected format: STEAM_X:Y:Z where X=0|1, Y=0|1, Z=number
 *
 * @param szSteamId2    SteamID2 string to validate
 * @return              true if valid format, false otherwise
 */
stock bool IsValidSteamID2(const char[] szSteamId2)
{
	int iLength = strlen(szSteamId2);
	if (iLength < STEAMID2_MIN_LENGTH || iLength > ACCOUNTID_LENGTH + 10)
	{
		return false;
	}

	if (StrContains(szSteamId2, "STEAM_", false) != 0)
	{
		return false;
	}

	int iSeparatorCount = 0;
	for (int i = 0; szSteamId2[i] != '\0'; i++)
	{
		if (szSteamId2[i] == ':')
			iSeparatorCount++;
	}
	if (iSeparatorCount != 2)
	{
		return false;
	}

	char szParts[3][32];
	if (ExplodeString(szSteamId2, ":", szParts, sizeof(szParts), sizeof(szParts[])) != 3)
		return false;

	if (!StrEqual(szParts[0], "STEAM_0", false) && !StrEqual(szParts[0], "STEAM_1", false))
		return false;
	if (!StrEqual(szParts[1], "0") && !StrEqual(szParts[1], "1"))
		return false;

	return SteamIDTools_IsSteamID2ZInRange(szParts[2]);
}

/**
 * Converts SteamID2 to AccountID
 * Format: STEAM_X:Y:Z -> AccountID = Z * 2 + Y
 *
 * @param szSteamId2    SteamID2 string (STEAM_X:Y:Z)
 * @return              AccountID, or 0 if invalid
 */
stock int SteamID2ToAccountID(const char[] szSteamId2)
{
	if (!IsValidSteamID2(szSteamId2))
	{
		return 0;
	}

	char szParts[3][32];
	if (ExplodeString(szSteamId2, ":", szParts, sizeof(szParts), sizeof(szParts[])) != 3)
	{
		return 0;
	}

	int iY;
	int iZ;
	if (!SteamIDTools_TryParseUInt32(szParts[1], iY)
		|| (iY != 0 && iY != 1)
		|| !SteamIDTools_TryParseUInt32(szParts[2], iZ)
		|| !SteamIDTools_IsSteamID2ZInRange(szParts[2]))
		return 0;

	int iHigh = iZ >> 15;
	int iLow = (iZ & 0x7fff) << 1;
	iLow |= iY;
	return (iHigh << 16) | iLow;
}

/**
 * Validates that the decimal SteamID2 Z component fits in 31 unsigned bits.
 *
 * @param szValue       Decimal component, with optional leading zeroes.
 * @return              True for values 0..2147483647, false otherwise.
 */
stock bool SteamIDTools_IsSteamID2ZInRange(const char[] szValue)
{
	int iValue;
	return SteamIDTools_TryParseUInt32(szValue, iValue) && iValue >= 0;
}

/**
 * Parses decimal digits in the range 0..4294967295 without int64 arithmetic.
 *
 * Leading zeroes are accepted within ACCOUNTID_LENGTH. The result retains the
 * uint32 bit pattern in a signed SourcePawn cell; negative cells can be valid.
 *
 * @param szValue       Decimal input, without signs or whitespace.
 * @param iValue        Receives the uint32 bit pattern, or zero on failure.
 * @return              True on success, false for invalid input or overflow.
 */
stock bool SteamIDTools_TryParseUInt32(const char[] szValue, int &iValue)
{
	iValue = 0;
	int iLength = strlen(szValue);
	if (iLength <= 0 || iLength > ACCOUNTID_LENGTH)
		return false;

	int iFirstDigit = 0;
	while (iFirstDigit < iLength && szValue[iFirstDigit] == '0')
		iFirstDigit++;
	if (iFirstDigit == iLength)
		return true;

	int iSignificantLength = iLength - iFirstDigit;
	if (iSignificantLength > 10)
		return false;
	if (iSignificantLength == 10)
	{
		for (int i = 0; i < 10; i++)
		{
			char cDigit = szValue[iFirstDigit + i];
			if (cDigit < '0' || cDigit > '9')
				return false;
			if (cDigit != ACCOUNTID_UINT32_MAX_STRING[i])
			{
				if (cDigit > ACCOUNTID_UINT32_MAX_STRING[i])
					return false;
				break;
			}
		}
	}

	int iHigh = 0;
	int iLow = 0;
	for (int i = iFirstDigit; i < iLength; i++)
	{
		if (szValue[i] < '0' || szValue[i] > '9')
			return false;

		int iExpandedLow = (iLow * 10) + (szValue[i] - '0');
		iHigh = (iHigh * 10) + (iExpandedLow / 65536);
		iLow = iExpandedLow % 65536;
	}

	iValue = (iHigh << 16) | iLow;
	return true;
}

/**
 * Parses a nonzero uint32 Steam AccountID.
 *
 * @param szValue       Decimal AccountID string.
 * @param iAccountId    Receives the AccountID bit pattern, or zero on failure.
 * @return              True for values 1..4294967295, false otherwise.
 */
stock bool SteamIDTools_TryParseAccountID(const char[] szValue, int &iAccountId)
{
	if (!SteamIDTools_TryParseUInt32(szValue, iAccountId))
		return false;
	return iAccountId != 0;
}

/**
 * Formats a nonzero AccountID as unsigned decimal without int64 arithmetic.
 *
 * @param iAccountId    AccountID bit pattern; a negative cell can be valid.
 * @param szBuffer      Destination buffer, unchanged on failure.
 * @param iMaxLength    Buffer capacity in bytes, including the null terminator.
 * @return              True on success, false for zero or insufficient capacity.
 */
stock bool SteamIDTools_FormatAccountID(int iAccountId, char[] szBuffer, int iMaxLength)
{
	if (iAccountId == 0 || iMaxLength <= 1)
		return false;

	int iHigh = (iAccountId >> 16) & 0xffff;
	int iLow = iAccountId & 0xffff;
	char szReversed[ACCOUNTID_LENGTH + 1];
	int iLength = 0;
	while (iHigh != 0 || iLow != 0)
	{
		if (iLength >= sizeof(szReversed) - 1)
			return false;

		int iRemainder = iHigh % 10;
		iHigh /= 10;
		int iCombined = (iRemainder * 65536) + iLow;
		iLow = iCombined / 10;
		szReversed[iLength++] = view_as<char>('0' + (iCombined % 10));
	}

	if (iLength + 1 > iMaxLength)
		return false;
	for (int i = 0; i < iLength; i++)
		szBuffer[i] = szReversed[iLength - i - 1];
	szBuffer[iLength] = '\0';
	return true;
}

/**
 * Adds two nonempty unsigned decimal strings without int64 arithmetic.
 *
 * The result can contain at most 63 digits. Leading zeroes are preserved.
 *
 * @param szBaseValue   First decimal operand.
 * @param szAddValue    Second decimal operand.
 * @param szResult      Destination buffer, unchanged on failure.
 * @param iMaxLength    Buffer capacity in bytes, including the null terminator.
 * @return              True on success, false for invalid input or capacity.
 */
stock bool SteamIDTools_AddDecimalStrings(const char[] szBaseValue, const char[] szAddValue, char[] szResult, int iMaxLength)
{
	if (iMaxLength <= 1)
		return false;
	if (szBaseValue[0] == '\0' || szAddValue[0] == '\0')
		return false;
	for (int i = 0; szBaseValue[i] != '\0'; i++)
	{
		if (szBaseValue[i] < '0' || szBaseValue[i] > '9')
			return false;
	}
	for (int i = 0; szAddValue[i] != '\0'; i++)
	{
		if (szAddValue[i] < '0' || szAddValue[i] > '9')
			return false;
	}

	int iBasePos = strlen(szBaseValue) - 1;
	int iAddPos = strlen(szAddValue) - 1;
	int iCarry = 0;
	int iReversedLength = 0;
	char szReversed[64];

	while (iBasePos >= 0 || iAddPos >= 0 || iCarry != 0)
	{
		if (iReversedLength >= 64 - 1)
			return false;

		int iDigit = iCarry;
		if (iBasePos >= 0)
			iDigit += szBaseValue[iBasePos--] - '0';
		if (iAddPos >= 0)
			iDigit += szAddValue[iAddPos--] - '0';

		szReversed[iReversedLength++] = view_as<char>('0' + (iDigit % 10));
		iCarry = iDigit / 10;
	}

	if (iReversedLength + 1 > iMaxLength)
		return false;

	for (int i = 0; i < iReversedLength; i++)
	{
		szResult[i] = szReversed[iReversedLength - i - 1];
	}

	szResult[iReversedLength] = '\0';
	return true;
}

/**
 * Converts a public individual SteamID64 to a nonzero uint32 AccountID.
 *
 * The subtraction uses decimal strings. Values above 2147483647 retain their
 * unsigned bit pattern in a negative SourcePawn cell; compare failures to zero.
 *
 * @param szSteamId64   Decimal SteamID64 string.
 * @param iAccountId    Receives the AccountID bit pattern, or zero on failure.
 * @return              True on success, false for invalid SteamID64 input.
 */
stock bool SteamIDTools_SteamID64ToAccountID(const char[] szSteamId64, int &iAccountId)
{
	iAccountId = 0;
	if (!IsValidSteamID64(szSteamId64))
		return false;

	char szReversed[STEAMID64_EXACT_LENGTH + 1];
	int iSourcePosition = strlen(szSteamId64) - 1;
	int iBasePosition = strlen(STEAMID64_BASE_STRING) - 1;
	int iBorrow = 0;
	int iLength = 0;
	while (iSourcePosition >= 0)
	{
		if (iLength >= sizeof(szReversed) - 1)
			return false;

		int iDigit = (szSteamId64[iSourcePosition--] - '0') - iBorrow;
		if (iBasePosition >= 0)
			iDigit -= STEAMID64_BASE_STRING[iBasePosition--] - '0';
		if (iDigit < 0)
		{
			iDigit += 10;
			iBorrow = 1;
		}
		else
		{
			iBorrow = 0;
		}
		szReversed[iLength++] = view_as<char>('0' + iDigit);
	}
	if (iBasePosition >= 0 || iBorrow != 0)
		return false;

	while (iLength > 1 && szReversed[iLength - 1] == '0')
		iLength--;

	char szAccountId[ACCOUNTID_LENGTH + 1];
	for (int i = 0; i < iLength; i++)
		szAccountId[i] = szReversed[iLength - i - 1];
	szAccountId[iLength] = '\0';
	return SteamIDTools_TryParseAccountID(szAccountId, iAccountId);
}

/**
 * Converts AccountID to SteamID2
 *
 * @param iAccountId    Nonzero uint32 AccountID bit pattern; may be negative.
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool AccountIDToSteamID2(int iAccountId, char[] szBuffer, int iMaxlen)
{
	if (iAccountId == 0 || iMaxlen <= 0)
	{
		return false;
	}

	int iY = iAccountId & 1;
	int iHigh = (iAccountId >> 16) & 0xffff;
	int iLow = iAccountId & 0xffff;
	int iZ = (iHigh >> 1) << 16;
	iZ |= (iLow >> 1) | ((iHigh & 1) << 15);

	char szResult[32];
	Format(szResult, sizeof(szResult), "STEAM_%d:%d:%d", STEAMIDTOOLS_SID2_UNIVERSE, iY, iZ);
	if (strlen(szResult) >= iMaxlen)
		return false;
	strcopy(szBuffer, iMaxlen, szResult);
	return true;
}

/**
 * Adds a nonnegative signed integer to an unsigned decimal string.
 *
 * @param szBaseValue   Base decimal value string
 * @param iAddValue     Signed value in the range 0..2147483647.
 * @param szResult      Output buffer
 * @param iMaxLength    Output buffer length
 * @return              true on success, false on failure
 */
stock bool SteamIDTools_AddDecimalStringInt(const char[] szBaseValue, int iAddValue, char[] szResult, int iMaxLength)
{
	if (iAddValue < 0)
		return false;

	char szAddValue[16];
	IntToString(iAddValue, szAddValue, sizeof(szAddValue));
	return SteamIDTools_AddDecimalStrings(szBaseValue, szAddValue, szResult, iMaxLength);
}

/**
 * Converts AccountID to SteamID64.
 *
 * @param iAccountId    Nonzero uint32 AccountID bit pattern; may be negative.
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool SteamIDTools_AccountIDToSteamID64(int iAccountId, char[] szBuffer, int iMaxlen)
{
	if (iAccountId == 0)
		return false;

	char szAccountId[ACCOUNTID_LENGTH + 1];
	if (!SteamIDTools_FormatAccountID(iAccountId, szAccountId, sizeof(szAccountId)))
		return false;
	return SteamIDTools_AddDecimalStrings(STEAMID64_BASE_STRING, szAccountId, szBuffer, iMaxlen);
}

/**
 * Validates if a SteamID64 is valid
 *
 * @param szSteamId64   SteamID64 string to validate
 * @return              true if valid, false otherwise
 */
stock bool IsValidSteamID64(const char[] szSteamId64)
{
	if (strlen(szSteamId64) < 17 || strlen(szSteamId64) > 17)
	{
		return false;
	}

	for (int i = 0; szSteamId64[i] != '\0'; i++)
	{
		if (!IsCharNumeric(szSteamId64[i]))
		{
			return false;
		}
	}

	if (strcmp(szSteamId64, STEAMID64_MIN_INDIVIDUAL_STRING, false) < 0)
	{
		return false;
	}

	if (strcmp(szSteamId64, STEAMID64_MAX_INDIVIDUAL_STRING, false) > 0)
	{
		return false;
	}

	return true;
}

/**
 * Validates if a SteamID3 is valid
 * Format: [U:1:XXXXXXXX]
 *
 * @param szSteamId3    SteamID3 string to validate
 * @return              true if valid, false otherwise
 */
stock bool IsValidSteamID3(const char[] szSteamId3)
{
	int iLength = strlen(szSteamId3);
	if (iLength < STEAMID3_MIN_LENGTH || iLength > ACCOUNTID_LENGTH + 6)
	{
		return false;
	}

	if (StrContains(szSteamId3, "[U:1:", false) != 0)
	{
		return false;
	}

	if (szSteamId3[iLength - 1] != ']')
	{
		return false;
	}

	char szAccountId[ACCOUNTID_LENGTH + 1];
	int iAccountLength = iLength - 6;
	for (int i = 0; i < iAccountLength; i++)
		szAccountId[i] = szSteamId3[i + 5];
	szAccountId[iAccountLength] = '\0';
	int iAccountId;
	return SteamIDTools_TryParseAccountID(szAccountId, iAccountId);
}

/**
 * Converts SteamID3 to AccountID
 * Format: [U:1:XXXXXXXX] -> XXXXXXXX
 *
 * @param szSteamId3    SteamID3 string
 * @return              AccountID, or 0 if invalid
 */
stock int SteamID3ToAccountID(const char[] szSteamId3)
{
	if (!IsValidSteamID3(szSteamId3))
	{
		return 0;
	}

	char szTemp[32];
	strcopy(szTemp, sizeof(szTemp), szSteamId3[5]);

	int iLen = strlen(szTemp);
	if (iLen > 0 && szTemp[iLen - 1] == ']')
	{
		szTemp[iLen - 1] = '\0';
	}

	int iAccountId;
	if (!SteamIDTools_TryParseAccountID(szTemp, iAccountId))
		return 0;
	return iAccountId;
}

/**
 * Converts AccountID to SteamID3
 *
 * @param iAccountId    Nonzero uint32 AccountID bit pattern; may be negative.
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool AccountIDToSteamID3(int iAccountId, char[] szBuffer, int iMaxlen)
{
	if (iAccountId == 0 || iMaxlen <= 0)
	{
		return false;
	}

	char szResult[32];
	Format(szResult, sizeof(szResult), "[U:1:%u]", iAccountId);
	if (strlen(szResult) >= iMaxlen)
		return false;
	strcopy(szBuffer, iMaxlen, szResult);
	return true;
}

/**
 * Builds a Steam community profile URL using a SteamID3 path.
 *
 * @param iAccountId    AccountID to convert
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool AccountIDToProfileURL_SID3(int iAccountId, char[] szBuffer, int iMaxlen)
{
	char szSteamId3[32];
	if (!AccountIDToSteamID3(iAccountId, szSteamId3, sizeof(szSteamId3)))
	{
		return false;
	}

	Format(szBuffer, iMaxlen, "%s%s", URL_SID, szSteamId3);
	return true;
}

/**
 * Builds a Steam community profile URL using a SteamID64 path.
 *
 * @param szSteamId64   SteamID64 string
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool SteamID64ToProfileURL(const char[] szSteamId64, char[] szBuffer, int iMaxlen)
{
	if (!IsValidSteamID64(szSteamId64))
	{
		return false;
	}

	Format(szBuffer, iMaxlen, "%s%s", URL_SID, szSteamId64);
	return true;
}

/**
 * Converts SteamID2 to SteamID3
 *
 * @param szSteamId2    SteamID2 string
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool SteamID2ToSteamID3(const char[] szSteamId2, char[] szBuffer, int iMaxlen)
{
	int iAccountId = SteamID2ToAccountID(szSteamId2);
	if (iAccountId == 0)
	{
		return false;
	}

	return AccountIDToSteamID3(iAccountId, szBuffer, iMaxlen);
}

/**
 * Converts SteamID3 to SteamID2
 *
 * @param szSteamId3    SteamID3 string
 * @param szBuffer      Buffer to store the result
 * @param iMaxlen       Maximum buffer length
 * @return              true on success, false on failure
 */
stock bool SteamID3ToSteamID2(const char[] szSteamId3, char[] szBuffer, int iMaxlen)
{
	int iAccountId = SteamID3ToAccountID(szSteamId3);
	if (iAccountId == 0)
	{
		return false;
	}

	return AccountIDToSteamID2(iAccountId, szBuffer, iMaxlen);
}

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

/**
 * Checks if a SteamID is a special case (BOT, PENDING, LAN)
 *
 * @param szSteamId     SteamID string to check
 * @return              true if it's a special case, false otherwise
 */
stock bool IsSteamIDSpecialCase(const char[] szSteamId)
{
	return (StrEqual(szSteamId, "BOT", false) || StrEqual(szSteamId, "STEAM_ID_PENDING", false) || StrEqual(szSteamId, "STEAM_ID_LAN", false));
}

/**
 * Automatically detects the format of a SteamID
 *
 * @param szSteamId     SteamID string to analyze
 * @return              SteamIDFormat enum value
 */
stock SteamIDFormat DetectSteamIDFormat(const char[] szSteamId)
{
	if (strlen(szSteamId) == 0)
	{
		return STEAMID_FORMAT_UNKNOWN;
	}

	if (IsSteamIDSpecialCase(szSteamId))
	{
		return STEAMID_FORMAT_SPECIAL;
	}

	if (StrContains(szSteamId, "STEAM_", false) == 0 && IsValidSteamID2(szSteamId))
	{
		return STEAMID_FORMAT_STEAMID2;
	}

	if (StrContains(szSteamId, "[U:1:", false) == 0 && IsValidSteamID3(szSteamId))
	{
		return STEAMID_FORMAT_STEAMID3;
	}

	if (strlen(szSteamId) == STEAMID64_EXACT_LENGTH && IsValidSteamID64(szSteamId))
	{
		return STEAMID_FORMAT_STEAMID64;
	}

	if (strlen(szSteamId) <= ACCOUNTID_LENGTH)
	{
		bool bIsNumeric = true;
		for (int i = 0; szSteamId[i] != '\0'; i++)
		{
			if (!IsCharNumeric(szSteamId[i]))
			{
				bIsNumeric = false;
				break;
			}
		}

		if (bIsNumeric)
		{
			int iAccountId;
			if (SteamIDTools_TryParseAccountID(szSteamId, iAccountId))
			{
				return STEAMID_FORMAT_ACCOUNTID;
			}
		}
	}

	return STEAMID_FORMAT_UNKNOWN;
}
