/*
	steamidtools.inc
	Copyright (C) 2025 AoC-Gamers
*/

#if defined _steamidtools_included
	#endinput
#endif
#define _steamidtools_included

#include <steamidtools_stock>

#define STEAMIDTOOLS_LIBRARY            "steamidtools"

#define STEAMIDTOOLS_MAX_ENDPOINT_LENGTH 32
#define STEAMIDTOOLS_MAX_REQUEST_LENGTH  1024
#define STEAMIDTOOLS_MAX_RESULT_LENGTH   4096
#define STEAMIDTOOLS_MAX_TAG_LENGTH      64

#define API_SID64toAID    "/SID64toAID"
#define API_SID64toSID2   "/SID64toSID2"
#define API_SID64toSID3   "/SID64toSID3"
#define API_AIDtoSID64    "/AIDtoSID64"
#define API_SID2toSID64   "/SID2toSID64"
#define API_SID3toSID64   "/SID3toSID64"
#define API_Health        "/health"

public SharedPlugin __pl_steamidtools =
{
	name = STEAMIDTOOLS_LIBRARY,
	file = "steamidtools.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_steamidtools_SetNTVOptional()
{
	MarkNativeAsOptional("SteamIDTools_IsProviderAvailable");
	MarkNativeAsOptional("SteamIDTools_GetBackendStatus");
	MarkNativeAsOptional("SteamIDTools_IsProviderReady");
	MarkNativeAsOptional("SteamIDTools_RequestHealthCheck");
	MarkNativeAsOptional("SteamIDTools_GetBackendStatusMessage");
	MarkNativeAsOptional("SteamIDTools_GetApiBaseUrl");
	MarkNativeAsOptional("SteamIDTools_RequestConversion");
	MarkNativeAsOptional("SteamIDTools_RequestBatch");
}
#endif

enum SteamIDToolsProvider
{
	SteamIDToolsProvider_Auto = 0,
	SteamIDToolsProvider_SteamWorks,
	SteamIDToolsProvider_System2
}

enum SteamIDToolsBackendStatus
{
	SteamIDToolsBackendStatus_Unknown = 0,
	SteamIDToolsBackendStatus_Online,
	SteamIDToolsBackendStatus_Offline
}

/**
 * Returns true when the SteamIDTools API plugin library is loaded.
 */
stock bool SteamIDTools_IsLibraryAvailable()
{
	return LibraryExists(STEAMIDTOOLS_LIBRARY);
}

/**
 * Returns true if the requested online provider is available in the API plugin.
 *
 * @param provider      Provider enum value
 * @return              true when the provider can execute HTTP requests
 */
native bool SteamIDTools_IsProviderAvailable(SteamIDToolsProvider provider);

/**
 * Returns the cached backend status for the selected provider.
 *
 * `Unknown` means no successful or failed health probe has been cached yet,
 * or the provider is currently unavailable.
 *
 * @param provider      Provider enum value
 * @return              Cached backend status enum
 */
native SteamIDToolsBackendStatus SteamIDTools_GetBackendStatus(SteamIDToolsProvider provider);

/**
 * Returns true when the provider extension is loaded and the backend is healthy.
 *
 * @param provider      Provider enum value
 * @return              true when the provider is ready for online requests
 */
native bool SteamIDTools_IsProviderReady(SteamIDToolsProvider provider);

/**
 * Triggers an async backend health check for one provider.
 *
 * @param provider      Provider enum value
 * @return              true when the health probe was queued
 */
native bool SteamIDTools_RequestHealthCheck(SteamIDToolsProvider provider);

/**
 * Returns the last cached backend health message for one provider.
 *
 * @param provider      Provider enum value
 * @param szBuffer      Output buffer
 * @param iMaxLen       Output buffer length
 * @return              true on success, false when the provider is invalid
 */
native bool SteamIDTools_GetBackendStatusMessage(SteamIDToolsProvider provider, char[] szBuffer, int iMaxLen);

/**
 * Gets the configured SteamIDTools backend base URL from the API plugin.
 *
 * @param szBuffer      Output buffer
 * @param iMaxLen       Output buffer length
 * @return              true on success, false otherwise
 */
native bool SteamIDTools_GetApiBaseUrl(char[] szBuffer, int iMaxLen);

/**
 * Queues a single online backend request.
 *
 * `SteamIDToolsProvider_Auto` means "auto". The API plugin selects the
 * best available provider, preferring a backend-ready transport and falling
 * back to any loaded provider when health has not been established yet.
 *
 * @param provider      Provider to use (`SteamWorks`, `system2`, or `Unknown` for auto)
 * @param szEndpoint    API endpoint, usually one of the `API_*` constants
 * @param szSteamId     Input value sent as `steamid=...`
 * @param szTag         Opaque caller tag returned in the completion forward
 * @return              Request id greater than zero if queued, otherwise zero
 */
native int SteamIDTools_RequestConversion(SteamIDToolsProvider provider, const char[] szEndpoint, const char[] szSteamId, const char[] szTag = "");

/**
 * Queues a batch online backend request.
 *
 * `SteamIDToolsProvider_Auto` means "auto". The API plugin selects the
 * best available provider, preferring a backend-ready transport and falling
 * back to any loaded provider when health has not been established yet.
 *
 * @param provider      Provider to use (`SteamWorks`, `system2`, or `Unknown` for auto)
 * @param szEndpoint    API endpoint, usually one of the `API_*` constants
 * @param szBatch       Comma-separated batch sent as `steamid=...`
 * @param szTag         Opaque caller tag returned in the completion forward
 * @return              Request id greater than zero if queued, otherwise zero
 */
native int SteamIDTools_RequestBatch(SteamIDToolsProvider provider, const char[] szEndpoint, const char[] szBatch, const char[] szTag = "");

/**
 * Fired when an online backend request finishes.
 *
 * @param iRequestId    Request id returned by the native that queued it
 * @param provider      Provider that executed the request
 * @param bSuccess      true on success, false on failure
 * @param bBatch        true for batch requests
 * @param szEndpoint    Endpoint used for the request
 * @param szInput       Original input sent to the backend
 * @param szResult      Response body on success, error text on failure
 * @param szTag         Opaque caller tag passed to the request native
 */
forward void SteamIDTools_OnRequestFinished(int iRequestId, SteamIDToolsProvider provider, bool bSuccess, bool bBatch, const char[] szEndpoint, const char[] szInput, const char[] szResult, const char[] szTag);

/**
 * Fired whenever the cached backend status changes for one provider.
 *
 * @param provider      Provider that changed
 * @param status        New cached backend status
 * @param szMessage     Latest status text or error message
 */
forward void SteamIDTools_OnBackendStatusChanged(SteamIDToolsProvider provider, SteamIDToolsBackendStatus status, const char[] szMessage);
