/**
 * @file l4d2_familyshare_manager.inc
 * @brief Borrower exceptions and connection enforcement queries.
 *
 * AccountIDs use unsigned 32-bit cell bit patterns. Use %u when formatting
 * them; zero is not a valid account identity.
 *
 * Define REQUIRE_PLUGIN for a required dependency. For optional use, undefine
 * it before including this file and check library/native availability before
 * calling a native.
 */

#if defined _l4d2_familyshare_manager_included_
	#endinput
#endif
#define _l4d2_familyshare_manager_included_

/** Runtime library registered by l4d2_familyshare_manager.smx. */
#define L4D2_FAMILYSHARE_MANAGER_LIBRARY "l4d2_familyshare_manager"

/**
 * Returns whether an AccountID has an enabled exception in the currently
 * loaded allowlist. The account does not need to be connected to the server.
 *
 * An exception applies only to this AccountID when it is the borrower. Adding
 * an owner does not allow other accounts borrowing that owner's license.
 * This query uses the last successfully loaded policy; it does not reload the
 * external allowlist file.
 *
 * @param accountId     Borrower AccountID to query.
 * @return              True if an enabled exception exists, false otherwise.
 *                      Zero and accounts absent from the loaded policy return
 *                      false. False alone does not imply a scheduled kick.
 */
native bool L4D2FamilyShareManager_IsAccountAllowed(int accountId);

/**
 * Returns the enforcement intent captured by manager for a client's current
 * connection when it processed the latest Family Sharing detection.
 *
 * A true result records the decision to schedule an expulsion. The deferred
 * kick rechecks the connection and policy, so this value does not confirm that
 * a kick occurred. Later policy changes do not recompute the captured value.
 *
 * @param client        Client index to query.
 * @return              Captured enforcement intent, or false if the client is
 *                      invalid, disconnected, or has no enforcement intent
 *                      recorded for the current connection.
 */
native bool L4D2FamilyShareManager_IsClientEnforced(int client);

public SharedPlugin __pl_l4d2_familyshare_manager =
{
	name = L4D2_FAMILYSHARE_MANAGER_LIBRARY,
	file = "l4d2_familyshare_manager.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_l4d2_familyshare_manager_SetNTVOptional()
{
	MarkNativeAsOptional("L4D2FamilyShareManager_IsAccountAllowed");
	MarkNativeAsOptional("L4D2FamilyShareManager_IsClientEnforced");
}
#endif
