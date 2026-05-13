#if defined _bansystem_access_included
	#endinput
#endif
#define _bansystem_access_included

/**
 * Queues an access ban mutation using an already resolved accountid.
 *
 * This native is synchronous from the caller perspective only in the sense that it returns
 * whether the request was accepted by the module. The actual database mutation is async.
 *
 * @param iAdmin        Admin client index, or 0 for server console.
 * @param iAccountId    Internal Steam account id to punish.
 * @param iLength       Ban length in minutes. 0 means permanent.
 * @param szReason      Ban reason.
 * @param szContext     Optional ban context.
 * @return              True if the request was accepted by the module.
 */
native bool BSAccess_AddBanByAccountId(int iAdmin, int iAccountId, int iLength = 0, const char[] szReason = "", const char[] szContext = "");

/**
 * Queues an access unban mutation using an already resolved accountid.
 *
 * This native is synchronous from the caller perspective only in the sense that it returns
 * whether the request was accepted by the module. The actual database mutation is async.
 *
 * @param iAdmin        Admin client index, or 0 for server console.
 * @param iAccountId    Internal Steam account id to unban.
 * @return              True if the request was accepted by the module.
 */
native bool BSAccess_RemoveBanByAccountId(int iAdmin, int iAccountId);

/**
 * Returns true when the access module database is ready to accept mutations.
 *
 * @return              True if the access database is ready.
 */
native bool BSAccess_IsDatabaseReady();

/**
 * Returns true when the module has resolved access detail for a connected client.
 *
 * @param iClient       Client index.
 * @return              True if access detail is resolved for the client.
 */
native bool BSAccess_HasResolvedDetail(int iClient);

/**
 * Returns true when the connected client currently has resolved access ban state loaded.
 *
 * @param iClient       Client index.
 * @return              True if the client has an active resolved access ban.
 */
native bool BSAccess_IsClientBanned(int iClient);

/**
 * Queues an asynchronous access-ban lookup using a Steam accountid.
 *
 * The result is returned through BSAccess_OnAccountIdBanCheck. A return value of
 * 0 means the request could not be queued.
 *
 * @param iAccountId    Internal Steam account id to check.
 * @return              Request id, or 0 on failure.
 */
native int BSAccess_RequestBanCheckByAccountId(int iAccountId);

/**
 * Queues an asynchronous access-ban info lookup using a Steam accountid.
 *
 * The result is returned through BSAccess_OnAccountIdBanInfo. A return value of
 * 0 means the request could not be queued.
 *
 * @param iAccountId    Internal Steam account id to inspect.
 * @return              Request id, or 0 on failure.
 */
native int BSAccess_RequestBanInfoByAccountId(int iAccountId);

/**
 * Returns the resolved accountid cached for a connected client, or 0 when unresolved.
 *
 * @param iClient       Client index.
 * @return              Resolved accountid, or 0 if unresolved.
 */
native int BSAccess_GetResolvedAccountId(int iClient);

/**
 * Fired immediately before the access module kicks a client with an active access ban.
 *
 * @param iClient       Client index that is about to be denied.
 * @param iAccountId    Internal Steam account id for the denied client.
 */
forward void BSAccess_OnClientDenied(int iClient, int iAccountId);

/**
 * Fired when an asynchronous accountid ban check requested through
 * BSAccess_RequestBanCheckByAccountId finishes.
 *
 * @param iRequestId    Request id returned by the native.
 * @param iAccountId    Account id that was checked.
 * @param bSuccess      True when the lookup completed without SQL errors.
 * @param bBanned       True when the account currently has an active access ban.
 */
forward void BSAccess_OnAccountIdBanCheck(int iRequestId, int iAccountId, bool bSuccess, bool bBanned);

/**
 * Fired when an asynchronous accountid ban info request requested through
 * BSAccess_RequestBanInfoByAccountId finishes.
 *
 * @param iRequestId    Request id returned by the native.
 * @param iAccountId    Account id that was checked.
 * @param bSuccess      True when the lookup completed without SQL errors.
 * @param bBanned       True when the account currently has an active access ban.
 * @param iBanLength    Active ban length in minutes. 0 means permanent or unavailable.
 */
forward void BSAccess_OnAccountIdBanInfo(int iRequestId, int iAccountId, bool bSuccess, bool bBanned, int iBanLength);

public SharedPlugin __pl_bansystem_access =
{
	name = "bansystem_access",
	file = "bansystem_access.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0
	#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_bansystem_access_SetNTVOptional()
{
	MarkNativeAsOptional("BSAccess_AddBanByAccountId");
	MarkNativeAsOptional("BSAccess_RemoveBanByAccountId");
	MarkNativeAsOptional("BSAccess_IsDatabaseReady");
	MarkNativeAsOptional("BSAccess_HasResolvedDetail");
	MarkNativeAsOptional("BSAccess_IsClientBanned");
	MarkNativeAsOptional("BSAccess_RequestBanCheckByAccountId");
	MarkNativeAsOptional("BSAccess_RequestBanInfoByAccountId");
	MarkNativeAsOptional("BSAccess_GetResolvedAccountId");
}
#endif