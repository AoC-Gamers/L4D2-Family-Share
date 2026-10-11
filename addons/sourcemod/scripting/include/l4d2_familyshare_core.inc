/**
 * @file l4d2_familyshare_core.inc
 * @brief Family Sharing detection, connection identity and optional profiles.
 *
 * AccountIDs use the complete unsigned 32-bit range, stored as SourcePawn cell
 * bit patterns. Use %u when formatting them; a negative signed value may still
 * represent a valid AccountID. Zero represents an unavailable identity.
 *
 * Define REQUIRE_PLUGIN for a required dependency. For optional use, undefine
 * it before including this file and check library/native availability before
 * calling a native.
 */

#if defined _l4d2_familyshare_core_included_
	#endinput
#endif
#define _l4d2_familyshare_core_included_

/** Runtime library registered by l4d2_familyshare_core.smx. */
#define L4D2_FAMILYSHARE_CORE_LIBRARY "l4d2_familyshare_core"

/**
 * Returns whether Family Sharing has been detected for a client's current
 * connection.
 *
 * @param client        Client index to query.
 * @return              True if the connected human client is currently marked
 *                      as a borrower, false otherwise. Invalid indices,
 *                      disconnected clients and bots return false.
 */
native bool L4D2FamilyShareCore_IsClientDetected(int client);

/**
 * Returns the borrower's AccountID recorded for a client's current connection.
 *
 * @param client        Client index to query.
 * @return              Borrower AccountID, or 0 if the client is invalid,
 *                      disconnected, a bot, or has no current detection.
 */
native int L4D2FamilyShareCore_GetClientBorrowerAccountId(int client);

/**
 * Returns the license owner's AccountID recorded for a client's current
 * connection. The owner does not need to be connected to the server.
 *
 * @param client        Borrower client index to query.
 * @return              Owner AccountID, or 0 if the client is invalid,
 *                      disconnected, a bot, or has no current detection.
 */
native int L4D2FamilyShareCore_GetClientOwnerAccountId(int client);

/**
 * Called after SteamWorks validates a borrowed license for a connected human
 * client. The core updates its connection state before calling this forward.
 * Duplicate validations of the currently recorded borrower/owner pair are
 * suppressed. Loading the core late does not replay earlier validations.
 *
 * Save the AccountIDs for deferred processing. A client index identifies only
 * the current connection and may be reused after a disconnect. The core does
 * not decide whether the borrower is allowed or should be expelled.
 *
 * @param client            Connected borrower client index.
 * @param borrowerAccountId  AccountID of the client using the borrowed license.
 * @param ownerAccountId     AccountID of the license owner, who may be offline.
 * @noreturn
 */
forward void L4D2FamilyShareCore_OnDetected(int client, int borrowerAccountId, int ownerAccountId);

/**
 * Requests optional profile enrichment for an AccountID. The account does not
 * need to be connected to the server.
 *
 * An accepted request completes through L4D2FamilyShareCore_OnProfileResolved.
 * Completion is always asynchronous, including when only local fallback data
 * is available. Requests survive client disconnects and map changes.
 *
 * Request IDs belong to the current core instance and may be reused after it
 * reloads. Consumers should discard their request correlation when the core
 * library is removed.
 *
 * @param accountId     Nonzero AccountID to resolve.
 * @return              Positive request ID if accepted, or 0 if accountId is 0
 *                      or the core is shutting down.
 */
native int L4D2FamilyShareCore_RequestProfile(int accountId);

/**
 * Called once when an accepted profile request completes, including fallback
 * completion after a provider failure or timeout.
 *
 * This is a global forward. Match both requestId and accountId to a request
 * owned by your plugin. The account may have disconnected since the request.
 * SteamID64 and profile URL are derived locally; the name may be empty when
 * unavailable. Profile data does not grant access or permissions.
 *
 * @param requestId     Request ID returned by RequestProfile in this core instance.
 * @param accountId     AccountID originally requested.
 * @param steamId64     Read-only decimal SteamID64 string.
 * @param personaName   Read-only display name, or an empty string if unavailable.
 * @param profileUrl    Read-only HTTPS Steam Community profile URL.
 * @noreturn
 */
forward void L4D2FamilyShareCore_OnProfileResolved(int requestId, int accountId,
	const char[] steamId64, const char[] personaName, const char[] profileUrl);

public SharedPlugin __pl_l4d2_familyshare_core =
{
	name = L4D2_FAMILYSHARE_CORE_LIBRARY,
	file = "l4d2_familyshare_core.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_l4d2_familyshare_core_SetNTVOptional()
{
	MarkNativeAsOptional("L4D2FamilyShareCore_IsClientDetected");
	MarkNativeAsOptional("L4D2FamilyShareCore_GetClientBorrowerAccountId");
	MarkNativeAsOptional("L4D2FamilyShareCore_GetClientOwnerAccountId");
	MarkNativeAsOptional("L4D2FamilyShareCore_RequestProfile");
}
#endif
