#if defined _l4d2_familyshare_included_
	#endinput
#endif
#define _l4d2_familyshare_included_

/**
 * Returns whether the specified in-game human client was detected using a
 * shared copy of the game.
 *
 * @param client			Client index to query.
 * @return			True if the client was detected as family sharing.
 */
native bool L4D2FamilyShare_IsClientDetected(int client);

/**
 * Returns the borrower account ID stored for the specified detected client.
 *
 * @param client			Client index to query.
 * @return			Borrower account ID, or 0 when unavailable.
 */
native int L4D2FamilyShare_GetClientBorrowerAccountId(int client);

/**
 * Returns the lender/owner account ID stored for the specified detected client.
 *
 * @param client			Client index to query.
 * @return			Owner account ID, or 0 when unavailable.
 */
native int L4D2FamilyShare_GetClientOwnerAccountId(int client);

/**
 * Returns whether the detected client is being enforced by this plugin.
 *
 * @param client			Client index to query.
 * @return			True when the detection is configured to kick.
 */
native bool L4D2FamilyShare_IsClientEnforced(int client);

/**
 * Called when a borrower is confirmed to be using a shared copy.
 *
 * The callback fires before the plugin kicks the client, allowing third-party
 * plugins to react while the client is still connected.
 *
 * @param client			Borrower client index.
 * @param borrowerAccountId	Borrower Steam account ID.
 * @param ownerAccountId		Owner Steam account ID.
 * @param enforced		1 when the plugin will enforce with a kick.
 */
forward void L4D2FamilyShare_OnDetected(int client, int borrowerAccountId, int ownerAccountId, bool enforced);

public SharedPlugin __pl_l4d2_familyshare =
{
	name = "l4d2_familyshare",
	file = "l4d2_familyshare.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_l4d2_familyshare_SetNTVOptional()
{
	MarkNativeAsOptional("L4D2FamilyShare_IsClientDetected");
	MarkNativeAsOptional("L4D2FamilyShare_GetClientBorrowerAccountId");
	MarkNativeAsOptional("L4D2FamilyShare_GetClientOwnerAccountId");
	MarkNativeAsOptional("L4D2FamilyShare_IsClientEnforced");
}
#endif