#if defined _SteamWorks_Included
 #endinput
#endif
#define _SteamWorks_Included

/* results from UserHasLicenseForApp */
enum EUserHasLicenseForAppResult
{
	k_EUserHasLicenseResultHasLicense = 0,					// User has a license for specified app
	k_EUserHasLicenseResultDoesNotHaveLicense = 1,			// User does not have a license for the specified app
	k_EUserHasLicenseResultNoAuth = 2,						// User has not been authenticated
};

/* General result codes */
enum EResult
{
	k_EResultOK	= 1,							// success
	k_EResultFail = 2,							// generic failure 
	k_EResultNoConnection = 3,					// no/failed network connection
//	k_EResultNoConnectionRetry = 4,				// OBSOLETE - removed
	k_EResultInvalidPassword = 5,				// password/ticket is invalid
	k_EResultLoggedInElsewhere = 6,				// same user logged in elsewhere
	k_EResultInvalidProtocolVer = 7,			// protocol version is incorrect
	k_EResultInvalidParam = 8,					// a parameter is incorrect
	k_EResultFileNotFound = 9,					// file was not found
	k_EResultBusy = 10,							// called method busy - action not taken
	k_EResultInvalidState = 11,					// called object was in an invalid state
	k_EResultInvalidName = 12,					// name is invalid
	k_EResultInvalidEmail = 13,					// email is invalid
	k_EResultDuplicateName = 14,				// name is not unique
	k_EResultAccessDenied = 15,					// access is denied
	k_EResultTimeout = 16,						// operation timed out
	k_EResultBanned = 17,						// VAC2 banned
	k_EResultAccountNotFound = 18,				// account not found
	k_EResultInvalidSteamID = 19,				// steamID is invalid
	k_EResultServiceUnavailable = 20,			// The requested service is currently unavailable
	k_EResultNotLoggedOn = 21,					// The user is not logged on
	k_EResultPending = 22,						// Request is pending (may be in process, or waiting on third party)
	k_EResultEncryptionFailure = 23,			// Encryption or Decryption failed
	k_EResultInsufficientPrivilege = 24,		// Insufficient privilege
	k_EResultLimitExceeded = 25,				// Too much of a good thing
	k_EResultRevoked = 26,						// Access has been revoked (used for revoked guest passes)
	k_EResultExpired = 27,						// License/Guest pass the user is trying to access is expired
	k_EResultAlreadyRedeemed = 28,				// Guest pass has already been redeemed by account, cannot be acked again
	k_EResultDuplicateRequest = 29,				// The request is a duplicate and the action has already occurred in the past, ignored this time
	k_EResultAlreadyOwned = 30,					// All the games in this guest pass redemption request are already owned by the user
	k_EResultIPNotFound = 31,					// IP address not found
	k_EResultPersistFailed = 32,				// failed to write change to the data store
	k_EResultLockingFailed = 33,				// failed to acquire access lock for this operation
	k_EResultLogonSessionReplaced = 34,
	k_EResultConnectFailed = 35,
	k_EResultHandshakeFailed = 36,
	k_EResultIOFailure = 37,
	k_EResultRemoteDisconnect = 38,
	k_EResultShoppingCartNotFound = 39,			// failed to find the shopping cart requested
	k_EResultBlocked = 40,						// a user didn't allow it
	k_EResultIgnored = 41,						// target is ignoring sender
	k_EResultNoMatch = 42,						// nothing matching the request found
	k_EResultAccountDisabled = 43,
	k_EResultServiceReadOnly = 44,				// this service is not accepting content changes right now
	k_EResultAccountNotFeatured = 45,			// account doesn't have value, so this feature isn't available
	k_EResultAdministratorOK = 46,				// allowed to take this action, but only because requester is admin
	k_EResultContentVersion = 47,				// A Version mismatch in content transmitted within the Steam protocol.
	k_EResultTryAnotherCM = 48,					// The current CM can't service the user making a request, user should try another.
	k_EResultPasswordRequiredToKickSession = 49,// You are already logged in elsewhere, this cached credential login has failed.
	k_EResultAlreadyLoggedInElsewhere = 50,		// You are already logged in elsewhere, you must wait
	k_EResultSuspended = 51,					// Long running operation (content download) suspended/paused
	k_EResultCancelled = 52,					// Operation canceled (typically by user: content download)
	k_EResultDataCorruption = 53,				// Operation canceled because data is ill formed or unrecoverable
	k_EResultDiskFull = 54,						// Operation canceled - not enough disk space.
	k_EResultRemoteCallFailed = 55,				// an remote call or IPC call failed
	k_EResultPasswordUnset = 56,				// Password could not be verified as it's unset server side
	k_EResultExternalAccountUnlinked = 57,		// External account (PSN, Facebook...) is not linked to a Steam account
	k_EResultPSNTicketInvalid = 58,				// PSN ticket was invalid
	k_EResultExternalAccountAlreadyLinked = 59,	// External account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first
	k_EResultRemoteFileConflict = 60,			// The sync cannot resume due to a conflict between the local and remote files
	k_EResultIllegalPassword = 61,				// The requested new password is not legal
	k_EResultSameAsPreviousValue = 62,			// new value is the same as the old one ( secret question and answer )
	k_EResultAccountLogonDenied = 63,			// account login denied due to 2nd factor authentication failure
	k_EResultCannotUseOldPassword = 64,			// The requested new password is not legal
	k_EResultInvalidLoginAuthCode = 65,			// account login denied due to auth code invalid
	k_EResultAccountLogonDeniedNoMail = 66,		// account login denied due to 2nd factor auth failure - and no mail has been sent
	k_EResultHardwareNotCapableOfIPT = 67,		// 
	k_EResultIPTInitError = 68,					// 
	k_EResultParentalControlRestricted = 69,	// operation failed due to parental control restrictions for current user
	k_EResultFacebookQueryError = 70,			// Facebook query returned an error
	k_EResultExpiredLoginAuthCode = 71,			// account login denied due to auth code expired
	k_EResultIPLoginRestrictionFailed = 72,
	k_EResultAccountLockedDown = 73,
	k_EResultAccountLogonDeniedVerifiedEmailRequired = 74,
	k_EResultNoMatchingURL = 75,
	k_EResultBadResponse = 76,					// parse failure, missing field, etc.
	k_EResultRequirePasswordReEntry = 77,		// The user cannot complete the action until they re-enter their password
	k_EResultValueOutOfRange = 78,				// the value entered is outside the acceptable range
	k_EResultUnexpectedError = 79,				// something happened that we didn't expect to ever happen
	k_EResultDisabled = 80,						// The requested service has been configured to be unavailable
	k_EResultInvalidCEGSubmission = 81,			// The set of files submitted to the CEG server are not valid !
	k_EResultRestrictedDevice = 82,				// The device being used is not allowed to perform this action
	k_EResultRegionLocked = 83,					// The action could not be complete because it is region restricted
	k_EResultRateLimitExceeded = 84,			// Temporary rate limit exceeded, try again later, different from k_EResultLimitExceeded which may be permanent
	k_EResultAccountLoginDeniedNeedTwoFactor = 85,	// Need two-factor code to login
	k_EResultItemDeleted = 86,					// The thing we're trying to access has been deleted
	k_EResultAccountLoginDeniedThrottle = 87,	// login attempt failed, try to throttle response to possible attacker
	k_EResultTwoFactorCodeMismatch = 88,		// two factor code mismatch
	k_EResultTwoFactorActivationCodeMismatch = 89,	// activation code for two-factor didn't match
	k_EResultAccountAssociatedToMultiplePartners = 90,	// account has been associated with multiple partners
	k_EResultNotModified = 91,					// data not modified
	k_EResultNoMobileDevice = 92,				// the account does not have a mobile device associated with it
	k_EResultTimeNotSynced = 93,				// the time presented is out of range or tolerance
	k_EResultSmsCodeFailed = 94,				// SMS code failure (no match, none pending, etc.)
	k_EResultAccountLimitExceeded = 95,			// Too many accounts access this resource
	k_EResultAccountActivityLimitExceeded = 96,	// Too many changes to this account
	k_EResultPhoneActivityLimitExceeded = 97,	// Too many changes to this phone
	k_EResultRefundToWallet = 98,				// Cannot refund to payment method, must use wallet
	k_EResultEmailSendFailure = 99,				// Cannot send an email
	k_EResultNotSettled = 100,					// Can't perform operation till payment has settled
	k_EResultNeedCaptcha = 101,					// Needs to provide a valid captcha
	k_EResultGSLTDenied = 102,					// a game server login token owned by this token's owner has been banned
	k_EResultGSOwnerDenied = 103,				// game server owner is denied for other reason (account lock, community ban, vac ban, missing phone)
	k_EResultInvalidItemType = 104				// the type of thing we were requested to act on is invalid
};

/* This enum is used in client API methods, do not re-number existing values. */
enum EHTTPMethod
{
	k_EHTTPMethodInvalid = 0,
	k_EHTTPMethodGET,
	k_EHTTPMethodHEAD,
	k_EHTTPMethodPOST,
	k_EHTTPMethodPUT,
	k_EHTTPMethodDELETE,
	k_EHTTPMethodOPTIONS,
	k_EHTTPMethodPATCH,

	// The remaining HTTP methods are not yet supported, per rfc2616 section 5.1.1 only GET and HEAD are required for 
	// a compliant general purpose server.  We'll likely add more as we find uses for them.

	// k_EHTTPMethodTRACE,
	// k_EHTTPMethodCONNECT
};


/* HTTP Status codes that the server can send in response to a request, see rfc2616 section 10.3 for descriptions
 of each of these. */
enum EHTTPStatusCode
{
	// Invalid status code (this isn't defined in HTTP, used to indicate unset in our code)
	k_EHTTPStatusCodeInvalid =					0,

	// Informational codes
	k_EHTTPStatusCode100Continue =				100,
	k_EHTTPStatusCode101SwitchingProtocols =	101,

	// Success codes
	k_EHTTPStatusCode200OK =					200,
	k_EHTTPStatusCode201Created =				201,
	k_EHTTPStatusCode202Accepted =				202,
	k_EHTTPStatusCode203NonAuthoritative =		203,
	k_EHTTPStatusCode204NoContent =				204,
	k_EHTTPStatusCode205ResetContent =			205,
	k_EHTTPStatusCode206PartialContent =		206,

	// Redirection codes
	k_EHTTPStatusCode300MultipleChoices =		300,
	k_EHTTPStatusCode301MovedPermanently =		301,
	k_EHTTPStatusCode302Found =					302,
	k_EHTTPStatusCode303SeeOther =				303,
	k_EHTTPStatusCode304NotModified =			304,
	k_EHTTPStatusCode305UseProxy =				305,
	//k_EHTTPStatusCode306Unused =				306, (used in old HTTP spec, now unused in 1.1)
	k_EHTTPStatusCode307TemporaryRedirect =		307,

	// Error codes
	k_EHTTPStatusCode400BadRequest =			400,
	k_EHTTPStatusCode401Unauthorized =			401, // You probably want 403 or something else. 401 implies you're sending a WWW-Authenticate header and the client can sent an Authorization header in response.
	k_EHTTPStatusCode402PaymentRequired =		402, // This is reserved for future HTTP specs, not really supported by clients
	k_EHTTPStatusCode403Forbidden =				403,
	k_EHTTPStatusCode404NotFound =				404,
	k_EHTTPStatusCode405MethodNotAllowed =		405,
	k_EHTTPStatusCode406NotAcceptable =			406,
	k_EHTTPStatusCode407ProxyAuthRequired =		407,
	k_EHTTPStatusCode408RequestTimeout =		408,
	k_EHTTPStatusCode409Conflict =				409,
	k_EHTTPStatusCode410Gone =					410,
	k_EHTTPStatusCode411LengthRequired =		411,
	k_EHTTPStatusCode412PreconditionFailed =	412,
	k_EHTTPStatusCode413RequestEntityTooLarge =	413,
	k_EHTTPStatusCode414RequestURITooLong =		414,
	k_EHTTPStatusCode415UnsupportedMediaType =	415,
	k_EHTTPStatusCode416RequestedRangeNotSatisfiable = 416,
	k_EHTTPStatusCode417ExpectationFailed =		417,
	k_EHTTPStatusCode4xxUnknown = 				418, // 418 is reserved, so we'll use it to mean unknown
	k_EHTTPStatusCode429TooManyRequests	=		429,

	// Server error codes
	k_EHTTPStatusCode500InternalServerError =	500,
	k_EHTTPStatusCode501NotImplemented =		501,
	k_EHTTPStatusCode502BadGateway =			502,
	k_EHTTPStatusCode503ServiceUnavailable =	503,
	k_EHTTPStatusCode504GatewayTimeout =		504,
	k_EHTTPStatusCode505HTTPVersionNotSupported = 505,
	k_EHTTPStatusCode5xxUnknown =				599,
};

/* list of possible return values from the ISteamGameCoordinator API */
enum EGCResults
{
	k_EGCResultOK = 0,
	k_EGCResultNoMessage = 1,			// There is no message in the queue
	k_EGCResultBufferTooSmall = 2,		// The buffer is too small for the requested message
	k_EGCResultNotLoggedOn = 3,			// The client is not logged onto Steam
	k_EGCResultInvalidMessage = 4,		// Something was wrong with the message being sent with SendMessage
};

/**
 * Checks whether VAC is enabled on the server.
 *
 * @return true if VAC is enabled, false otherwise
 */
native bool SteamWorks_IsVACEnabled();

/**
 * Gets the server public IP address.
 *
 * @param ipaddr     Array that receives the 4 IP bytes
 * @return           true if retrieved successfully, false on error
 */
native bool SteamWorks_GetPublicIP(int ipaddr[4]);

/**
 * Gets the server public IP address as a numeric value.
 *
 * @return           Public IP in numeric cell format
 */
native int SteamWorks_GetPublicIPCell();

/**
 * Checks whether the SteamWorks extension is loaded.
 *
 * @return           true if loaded, false otherwise
 */
native bool SteamWorks_IsLoaded();

/**
 * Sets custom game data visible to the server browser.
 *
 * @param data       Key/value data string
 * @return           true if set successfully
 */
native bool SteamWorks_SetGameData(const char[] data);

/**
 * Sets the custom game description.
 *
 * @param description Description string
 * @return           true if set successfully
 */
native bool SteamWorks_SetGameDescription(const char[] description);

/**
 * Sets the map name shown in Steam.
 *
 * @param mapName    Map name string
 * @return           true if set successfully
 */
native bool SteamWorks_SetMapName(const char[] mapName);

/**
 * Checks whether the server is connected to Steam.
 *
 * @return           true if connected, false otherwise
 */
native bool SteamWorks_IsConnected();

/**
 * Sets a custom rule visible from the lobby or browser.
 *
 * @param key        Rule key
 * @param value      Rule value
 * @return           true if set successfully
 */
native bool SteamWorks_SetRule(const char[] key, const char[] value);

/**
 * Clears all previously set rules.
 *
 * @return           true if all rules were removed
 */
native bool SteamWorks_ClearRules();

/**
 * Checks whether a connected client belongs to a specific Steam group.
 *
 * @param client     Connected client index (1-64)
 * @param groupid    Steam group ID to check
 * @return           true if the client is a member, false otherwise or on error
 */
native bool SteamWorks_GetUserGroupStatus(int client, int groupid);

/**
 * Checks whether a specific SteamID belongs to a Steam group.
 *
 * @param authid     Player SteamID in numeric format (AuthID/SteamID64)
 * @param groupid    Steam group ID to check
 * @return           true if the user belongs to the group, false otherwise or on error
 */
native bool SteamWorks_GetUserGroupStatusAuthID(int authid, int groupid);

/**
 * Checks whether a connected client owns a license for a specific application.
 *
 * @param client     Connected client index
 * @param app        Game AppID (for example, 550 for L4D2)
 * @return           EUserHasLicenseForAppResult value
 */
native EUserHasLicenseForAppResult SteamWorks_HasLicenseForApp(int client, int app);

/**
 * Checks whether a specific SteamID owns a license for an application.
 *
 * @param authid     Player SteamID (AuthID or SteamID64)
 * @param app        Game AppID
 * @return           EUserHasLicenseForAppResult value
 */
native EUserHasLicenseForAppResult SteamWorks_HasLicenseForAppId(int authid, int app);

/**
 * Gets the client SteamID as a string.
 *
 * @param client     Connected client index
 * @param steamId    Buffer that receives the SteamID string
 * @param length     Maximum buffer length
 * @return           Number of characters written to steamId (excluding the null terminator)
 */
native int SteamWorks_GetClientSteamID(int client, char[] steamId, int length);

/**
 * Requests a player's stats from Steam using their SteamID (AuthID).
 *
 * @param authid     Player SteamID (AuthID or SteamID64)
 * @param appid      Game AppID (for example, 550 for L4D2)
 * @return           true if the request was sent successfully
 */
native bool SteamWorks_RequestStatsAuthID(int authid, int appid);

/**
 * Requests stats from Steam for a connected client.
 *
 * @param client     Client index
 * @param appid      Game AppID
 * @return           true if the request was sent successfully
 */
native bool SteamWorks_RequestStats(int client, int appid);

/**
 * Gets the integer value of a client stat.
 *
 * @param client     Client index
 * @param key        Stat name to query
 * @param value      Variable that receives the value
 * @return           true if the stat was retrieved successfully
 */
native bool SteamWorks_GetStatCell(int client, const char[] key, int &value);

/**
 * Gets the integer value of a stat using the player's SteamID.
 *
 * @param authid     Player SteamID (AuthID or SteamID64)
 * @param key        Stat name to query
 * @param value      Variable that receives the value
 * @return           true if the stat was retrieved successfully
 */
native bool SteamWorks_GetStatAuthIDCell(int authid, const char[] key, int &value);

/**
 * Gets the floating-point value of a client stat.
 *
 * @param client     Client index
 * @param key        Stat name to query
 * @param value      Variable that receives the float value
 * @return           true if the stat was retrieved successfully
 */
native bool SteamWorks_GetStatFloat(int client, const char[] key, float &value);

/**
 * Gets the floating-point value of a stat using the player's SteamID.
 *
 * @param authid     Player SteamID (AuthID or SteamID64)
 * @param key        Stat name to query
 * @param value      Variable that receives the float value
 * @return           true if the stat was retrieved successfully
 */
native bool SteamWorks_GetStatAuthIDFloat(int authid, const char[] key, float &value);

/**
 * Creates a new HTTP request.
 *
 * @param method     HTTP method to use (GET, POST, etc.)
 * @param url        Target URL for the request
 * @return           Handle to the created request, or INVALID_HANDLE on failure
 */
native Handle SteamWorks_CreateHTTPRequest(EHTTPMethod method, const char[] url);

/**
 * Sets contextual values associated with an HTTP request for later callbacks.
 *
 * @param request    HTTP request handle
 * @param data1      Custom value (for example, user ID or reference)
 * @param data2      Optional secondary value
 * @return           true if the values were assigned successfully
 */
native bool SteamWorks_SetHTTPRequestContextValue(Handle request, any data1, any data2=0);

/**
 * Sets the network activity timeout for an HTTP request.
 *
 * @param request    HTTP request handle
 * @param timeout    Maximum time in seconds before considering the request failed
 * @return           true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestNetworkActivityTimeout(Handle request, int timeout);

/**
 * Adds or replaces an HTTP header on the request.
 *
 * @param request    HTTP request handle
 * @param name       Header name (for example, "Content-Type")
 * @param value      Header value
 * @return           true if set successfully
 */
native bool SteamWorks_SetHTTPRequestHeaderValue(Handle request, const char[] name, const char[] value);

/**
 * Adds a GET or POST parameter to the HTTP request.
 *
 * @param request    HTTP request handle
 * @param name       Parameter name
 * @param value      Parameter value
 * @return           true if added successfully
 */
native bool SteamWorks_SetHTTPRequestGetOrPostParameter(Handle request, const char[] name, const char[] value);

/**
 * Sets the User-Agent string for the HTTP request.
 *
 * @param request         HTTP request handle
 * @param userAgentInfo   User-Agent string to use
 * @return                true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestUserAgentInfo(Handle request, const char[] userAgentInfo);

/**
 * Sets whether the HTTP request must require a verified SSL certificate.
 *
 * @param request                  HTTP request handle
 * @param requireVerifiedCertificate  true to require a valid certificate, false to allow any certificate
 * @return                         true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestRequiresVerifiedCertificate(Handle request, bool requireVerifiedCertificate);

/**
 * Sets the absolute timeout for the HTTP request in milliseconds.
 *
 * @param request          HTTP request handle
 * @param timeoutMs        Maximum time in milliseconds before canceling the request
 * @return                 true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestAbsoluteTimeoutMS(Handle request, int timeoutMs);


/**
 * Callback fired when an HTTP request sent with SteamWorks completes.
 *
 * This typeset defines a function that is called when the request finishes,
 * whether successfully or with an error. It can be used to process the result,
 * read the response body, and so on.
 */
typeset SteamWorksHTTPRequestCompleted
{
	/**
	 * Basic callback without contextual data.
	 *
	 * @param request             HTTP request handle
	 * @param failed              true if a network or internal error occurred
	 * @param requestSuccessful   true if the request completed successfully (it may still return HTTP errors)
	 * @param statusCode          HTTP response code (200, 404, 500, etc.)
	 */
	function void (Handle request, bool failed, bool requestSuccessful, EHTTPStatusCode statusCode);

	/**
	 * Callback with one custom contextual value (data1).
	 *
	 * @param request             HTTP request handle
	 * @param failed              true if an error occurred
	 * @param requestSuccessful   true if the request completed
	 * @param statusCode          HTTP status code
	 * @param data1               User-defined contextual value (for example, a player identifier)
	 */
	function void (Handle request, bool failed, bool requestSuccessful, EHTTPStatusCode statusCode, any data1);

	/**
	 * Callback with two custom contextual values (data1 and data2).
	 *
	 * @param request             HTTP request handle
	 * @param failed              true if an error occurred
	 * @param requestSuccessful   true if the request completed
	 * @param statusCode          HTTP status code
	 * @param data1               First contextual value
	 * @param data2               Second contextual value
	 */
	function void (Handle request, bool failed, bool requestSuccessful, EHTTPStatusCode statusCode, any data1, any data2);
};

/**
 * Callback fired when HTTP headers are received for a request.
 *
 * This event occurs before the response body is received.
 * It is useful when inspecting or processing headers individually.
 */
typeset SteamWorksHTTPHeadersReceived
{
	/**
	 * Basic callback without contextual data.
	 *
	 * @param request    HTTP request handle
	 * @param failed     true if a network error occurred while receiving headers
	 */
	function void (Handle request, bool failed);

	/**
	 * Callback with one custom contextual value (data1).
	 *
	 * @param request    HTTP request handle
	 * @param failed     true if an error occurred while receiving headers
	 * @param data1      User-defined contextual value
	 */
	function void (Handle request, bool failed, any data1);

	/**
	 * Callback with two custom contextual values (data1 and data2).
	 *
	 * @param request    HTTP request handle
	 * @param failed     true if an error occurred while receiving headers
	 * @param data1      First contextual value
	 * @param data2      Second contextual value
	 */
	function void (Handle request, bool failed, any data1, any data2);
};

/**
 * Callback fired when a chunk of HTTP response body data is received.
 *
 * This event is useful when using streamed responses with SteamWorks,
 * allowing data to be processed incrementally as it arrives.
 */
typeset SteamWorksHTTPDataReceived
{
	/**
	 * Basic callback without contextual data.
	 *
	 * @param request         HTTP request handle
	 * @param failed          true if an error occurred while receiving data
	 * @param offset          Byte offset where this chunk starts
	 * @param bytesReceived   Number of bytes received in this chunk
	 */
	function void (Handle request, bool failed, int offset, int bytesReceived);

	/**
	 * Callback with one custom contextual value (data1).
	 *
	 * @param request         HTTP request handle
	 * @param failed          true if an error occurred
	 * @param offset          Offset where the chunk was received
	 * @param bytesReceived   Size of the received chunk
	 * @param data1           Custom contextual value
	 */
	function void (Handle request, bool failed, int offset, int bytesReceived, any data1);

	/**
	 * Callback with two custom contextual values (data1 and data2).
	 *
	 * @param request         HTTP request handle
	 * @param failed          true if an error occurred
	 * @param offset          Offset where the chunk was received
	 * @param bytesReceived   Size of the received chunk
	 * @param data1           First contextual value
	 * @param data2           Second contextual value
	 */
	function void (Handle request, bool failed, int offset, int bytesReceived, any data1, any data2);
};


/**
 * Callback fired when the full HTTP response body is received.
 *
 * This typeset defines how to handle the content once downloaded,
 * either as a text string or as binary data.
 */
typeset SteamWorksHTTPBodyCallback
{
	/**
	 * Basic callback that receives the body as text.
	 *
	 * @param data      Text string containing the full response body
	 */
	function void (const char[] data);

	/**
	 * Callback that receives the body as text plus a contextual value.
	 *
	 * @param data      Text string containing the response body
	 * @param value     User-defined contextual value
	 */
	function void (const char[] data, any value);

	/**
	 * Callback that receives binary data as an integer array.
	 *
	 * @param data      Binary data array (bytes)
	 * @param value     User-defined contextual value
	 * @param datalen   Length of the received data array
	 */
	function void (const int[] data, any value, int datalen);
};

/**
 * Assigns callbacks used to handle HTTP request events.
 *
 * @param request     HTTP request handle
 * @param onCompleted Callback invoked when the request completes
 * @param onHeaders   Callback invoked when headers are received (optional)
 * @param onData      Callback invoked for streamed response data (optional)
 * @param caller      Plugin that owns the callback (null = current plugin)
 * @return            true if the callbacks were configured successfully
 */
native bool SteamWorks_SetHTTPCallbacks(
	Handle request,
	SteamWorksHTTPRequestCompleted onCompleted = INVALID_FUNCTION,
	SteamWorksHTTPHeadersReceived onHeaders = INVALID_FUNCTION,
	SteamWorksHTTPDataReceived onData = INVALID_FUNCTION,
	Handle caller = null
);

/**
 * Sends a previously created HTTP request.
 *
 * @param request     HTTP request handle
 * @return            true if the request was sent successfully
 */
native bool SteamWorks_SendHTTPRequest(Handle request);

/**
 * Sends an HTTP request and allows the body to be received in chunks (streaming).
 *
 * @param request     HTTP request handle
 * @return            true if the request was sent successfully
 */
native bool SteamWorks_SendHTTPRequestAndStreamResponse(Handle request);

/**
 * Defers sending the HTTP request until it is processed later.
 *
 * @param request     HTTP request handle
 * @return            true if the request was deferred successfully
 */
native bool SteamWorks_DeferHTTPRequest(Handle request);

/**
 * Prioritizes this HTTP request over others in the queue.
 *
 * @param request     HTTP request handle
 * @return            true if prioritized successfully
 */
native bool SteamWorks_PrioritizeHTTPRequest(Handle request);

/**
 * Gets the size of a specific header in the HTTP response.
 *
 * @param request     HTTP request handle
 * @param header      Header name (for example: "Content-Type")
 * @param size        Variable that receives the header size
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPResponseHeaderSize(Handle request, const char[] header, int &size);

/**
 * Gets the value of a specific header from the HTTP response.
 *
 * @param request     HTTP request handle
 * @param header      Header name
 * @param value       Destination buffer that receives the value
 * @param size        Maximum value buffer length
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPResponseHeaderValue(Handle request, const char[] header, char[] value, int size);

/**
 * Gets the total size of the HTTP response body.
 *
 * @param request     HTTP request handle
 * @param size        Variable that receives the size in bytes
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPResponseBodySize(Handle request, int &size);

/**
 * Gets the full HTTP response body as raw bytes.
 *
 * @param request     HTTP request handle
 * @param body        Buffer that receives the response bytes
 * @param length      Maximum number of bytes to read
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPResponseBodyData(Handle request, char[] body, int length);

/**
 * Gets the HTTP response body as a null-terminated string.
 *
 * This helper is intended for textual bodies such as JSON, HTML, or plain text.
 * It reads up to `length - 1` bytes and always appends a null terminator.
 * If the response body is larger than the provided buffer, the string will be truncated.
 *
 * This native should not be used for binary payloads.
 *
 * @param request     HTTP request handle
 * @param body        Buffer that receives the string
 * @param length      Maximum buffer length, including the null terminator
 * @param written     Number of characters written, excluding the null terminator
 * @param truncated   Set to true if the body did not fit in the buffer
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPResponseBodyString(Handle request, char[] body, int length, int &written, bool &truncated);

/**
 * Gets a fragment of the HTTP response body (for manual streaming).
 *
 * @param request     HTTP request handle
 * @param offset      Offset from which to start reading the body
 * @param body        Buffer that receives the fragment
 * @param length      Fragment size to read
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPStreamingResponseBodyData(Handle request, int offset, char[] body, int length);

/**
 * Gets the current download progress of an HTTP request.
 *
 * @param request     HTTP request handle
 * @param percent     Variable that receives the downloaded percentage (0.0 to 100.0)
 * @return            true if retrieved successfully
 */
native bool SteamWorks_GetHTTPDownloadProgressPct(Handle request, float &percent);

/**
 * Checks whether an HTTP request was canceled because it exceeded the time limit.
 *
 * @param request         HTTP request handle
 * @param wasTimedOut     Variable set to true if a timeout occurred
 * @return                true if checked successfully
 */
native bool SteamWorks_GetHTTPRequestWasTimedOut(Handle request, bool &wasTimedOut);

/**
 * Sets the body of a POST request manually.
 *
 * @param request        HTTP request handle
 * @param contentType    Content type (for example: "application/json")
 * @param body           Body data (raw content)
 * @param bodyLength     Length of the data to send
 * @return               true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestRawPostBody(Handle request, const char[] contentType, const char[] body, int bodyLength);

/**
 * Sets the body of a POST request from a file.
 *
 * @param request        HTTP request handle
 * @param contentType    Content type (for example: "application/octet-stream")
 * @param fileName       Local file path to use as the body
 * @return               true if configured successfully
 */
native bool SteamWorks_SetHTTPRequestRawPostBodyFromFile(Handle request, const char[] contentType, const char[] fileName);

/**
 * Attaches a callback that handles the HTTP response body once it is complete.
 *
 * @param request     HTTP request handle
 * @param callback    Function to execute when the body is received
 * @param data        Optional custom contextual value
 * @param plugin      Plugin that owns the callback (null for the current one)
 * @return            true if configured successfully
 */
native bool SteamWorks_GetHTTPResponseBodyCallback(Handle request, SteamWorksHTTPBodyCallback callback, any data = 0, Handle plugin = null);

/**
 * Writes the HTTP response body to a file on disk.
 *
 * @param request     HTTP request handle
 * @param fileName    Path to the file where the content will be written
 * @return            true if written successfully
 */
native bool SteamWorks_WriteHTTPResponseBodyToFile(Handle request, const char[] fileName);

/**
 * Event fired when a client is validated through Steam (legacy version, replaced by SteamWorks_OnValidateClient).
 *
 * @param ownerauthid   SteamID of the real game owner (may differ when Family Sharing is used)
 * @param authid        SteamID of the connecting client
 */
forward void SW_OnValidateClient(int ownerauthid, int authid);

/**
 * Event fired when Steam successfully validates a client.
 *
 * @param ownerauthid   SteamID of the real game owner
 * @param authid        SteamID of the connected client
 *
 * This event is useful for detecting Family Sharing cases (if ownerauthid != authid).
 */
forward void SteamWorks_OnValidateClient(int ownerauthid, int authid);

/**
 * Event fired when the server connects successfully to Steam.
 *
 * This can be used to run tasks that depend on Steam connectivity,
 * such as license validation or stats initialization.
 */
forward void SteamWorks_SteamServersConnected();

/**
 * Event fired when the server fails to connect to Steam.
 *
 * @param result        Error code that occurred (EResult)
 *
 * This may happen due to network issues, an incompatible steamclient, VAC being disabled, and so on.
 */
forward void SteamWorks_SteamServersConnectFailure(EResult result);

/**
 * Event fired when the server loses its Steam connection.
 *
 * @param result        Disconnect reason (EResult)
 *
 * Useful for logging or warning the server administrator.
 */
forward void SteamWorks_SteamServersDisconnected(EResult result);

/**
 * Event fired when Steam requests the server to restart.
 *
 * This can happen due to IP changes or other Steam-side conditions.
 *
 * @return Plugin_Handled to prevent the restart, Plugin_Continue to allow it
 */
forward Action SteamWorks_RestartRequested();

/**
 * Event fired when Steam requests a Game Server Token (if the game requires it).
 *
 * @param token      Buffer where the token should be written
 * @param maxlen     Maximum buffer length
 *
 * Note: L4D2 usually does not require a token, but this forward is useful for games that do.
 */
forward void SteamWorks_TokenRequested(char[] token, int maxlen);

/**
 * Event fired when the group status of a client is resolved for a Steam group.
 *
 * @param authid     SteamID of the client
 * @param groupid    Queried Steam group ID
 * @param isMember   true if the client is a member of the group
 * @param isOfficer  true if the client is an officer of the group
 */
forward void SteamWorks_OnClientGroupStatus(int authid, int groupid, bool isMember, bool isOfficer);

/**
 * Sends a binary message to the game's Game Coordinator.
 *
 * @param messageType Message type defined by the game
 * @param data        Message data
 * @param dataLength  Data length
 * @return           Send result (EGCResults)
 */
forward EGCResults SteamWorks_GCSendMessage(int messageType, const char[] data, int dataLength);

/**
 * Event fired when a message is received from the Game Coordinator.
 *
 * @param dataLength Size of the message available for reading
 */
forward void SteamWorks_GCMsgAvailable(int dataLength);

/**
 * Retrieves a message received from the Game Coordinator.
 *
 * @param messageType       Variable that receives the message type
 * @param destination       Buffer that receives the message contents
 * @param destinationLength Maximum buffer size
 * @param messageSize       Variable that receives the actual size of the received message
 * @return             Operation result (EGCResults)
 */
forward EGCResults SteamWorks_GCRetrieveMessage(int messageType, const char[] destination, int destinationLength, int messageSize);

/**
 * Sends a message to the Game Coordinator from a native function.
 *
 * @param messageType Message type defined by the game
 * @param data        Message data
 * @param dataLength  Data length
 * @return           Send result (EGCResults)
 */
native EGCResults SteamWorks_SendMessageToGC(int messageType, const char[] data, int dataLength);

public Extension __ext_SteamWorks = 
{
	name = "SteamWorks",
	file = "steamworks.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public void __ext_SteamWorks_SetNTVOptional()
{
	MarkNativeAsOptional("SteamWorks_IsVACEnabled");
	MarkNativeAsOptional("SteamWorks_GetPublicIP");
	MarkNativeAsOptional("SteamWorks_GetPublicIPCell");
	MarkNativeAsOptional("SteamWorks_IsLoaded");
	MarkNativeAsOptional("SteamWorks_SetGameData");
	MarkNativeAsOptional("SteamWorks_SetGameDescription");
	MarkNativeAsOptional("SteamWorks_IsConnected");
	MarkNativeAsOptional("SteamWorks_SetRule");
	MarkNativeAsOptional("SteamWorks_ClearRules");
	MarkNativeAsOptional("SteamWorks_GetUserGroupStatus");
	MarkNativeAsOptional("SteamWorks_GetUserGroupStatusAuthID");

	MarkNativeAsOptional("SteamWorks_HasLicenseForApp");
	MarkNativeAsOptional("SteamWorks_HasLicenseForAppId");
	MarkNativeAsOptional("SteamWorks_GetClientSteamID");

	MarkNativeAsOptional("SteamWorks_RequestStatsAuthID");
	MarkNativeAsOptional("SteamWorks_RequestStats");
	MarkNativeAsOptional("SteamWorks_GetStatCell");
	MarkNativeAsOptional("SteamWorks_GetStatAuthIDCell");
	MarkNativeAsOptional("SteamWorks_GetStatFloat");
	MarkNativeAsOptional("SteamWorks_GetStatAuthIDFloat");

	MarkNativeAsOptional("SteamWorks_SendMessageToGC");

	MarkNativeAsOptional("SteamWorks_CreateHTTPRequest");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestContextValue");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestNetworkActivityTimeout");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestHeaderValue");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestGetOrPostParameter");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestUserAgentInfo");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestRequiresVerifiedCertificate");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestAbsoluteTimeoutMS");

	MarkNativeAsOptional("SteamWorks_SetHTTPCallbacks");
	MarkNativeAsOptional("SteamWorks_SendHTTPRequest");
	MarkNativeAsOptional("SteamWorks_SendHTTPRequestAndStreamResponse");
	MarkNativeAsOptional("SteamWorks_DeferHTTPRequest");
	MarkNativeAsOptional("SteamWorks_PrioritizeHTTPRequest");
	MarkNativeAsOptional("SteamWorks_GetHTTPResponseHeaderSize");
	MarkNativeAsOptional("SteamWorks_GetHTTPResponseHeaderValue");
	MarkNativeAsOptional("SteamWorks_GetHTTPResponseBodySize");
	MarkNativeAsOptional("SteamWorks_GetHTTPResponseBodyData");
	MarkNativeAsOptional("SteamWorks_GetHTTPResponseBodyString");
	MarkNativeAsOptional("SteamWorks_GetHTTPStreamingResponseBodyData");
	MarkNativeAsOptional("SteamWorks_GetHTTPDownloadProgressPct");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestRawPostBody");
	MarkNativeAsOptional("SteamWorks_SetHTTPRequestRawPostBodyFromFile");

	MarkNativeAsOptional("SteamWorks_GetHTTPResponseBodyCallback");
	MarkNativeAsOptional("SteamWorks_WriteHTTPResponseBodyToFile");
}
#endif
