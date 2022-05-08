#pragma once

// results from UserHasLicenseForApp
enum EUserHasLicenseForAppResult {
    k_EUserHasLicenseResultHasLicense = 0,         // User has a license for specified app
    k_EUserHasLicenseResultDoesNotHaveLicense = 1, // User does not have a license for the specified app
    k_EUserHasLicenseResultNoAuth = 2,             // User has not been authenticated
};

typedef uint32_t SteamInventoryResult_t;
typedef uint64_t SteamItemInstanceID_t;
typedef uint32_t SteamItemDef_t;

struct SteamItemDetails_t {
    SteamItemInstanceID_t m_itemId;
    uint32_t m_iDefinition;
    uint16_t m_unQuantity;
    uint16_t m_unFlags; // see ESteamItemFlags
};

enum EResult {
    k_EResultNone = 0,                            // no result
    k_EResultOK = 1,                            // success
    k_EResultFail = 2,                            // generic failure
    k_EResultNoConnection = 3,                    // no/failed network connection
//	k_EResultNoConnectionRetry = 4,				// OBSOLETE - removed
    k_EResultInvalidPassword = 5,                // password/ticket is invalid
    k_EResultLoggedInElsewhere = 6,                // same user logged in elsewhere
    k_EResultInvalidProtocolVer = 7,            // protocol version is incorrect
    k_EResultInvalidParam = 8,                    // a parameter is incorrect
    k_EResultFileNotFound = 9,                    // file was not found
    k_EResultBusy = 10,                            // called method busy - action not taken
    k_EResultInvalidState = 11,                    // called object was in an invalid state
    k_EResultInvalidName = 12,                    // name is invalid
    k_EResultInvalidEmail = 13,                    // email is invalid
    k_EResultDuplicateName = 14,                // name is not unique
    k_EResultAccessDenied = 15,                    // access is denied
    k_EResultTimeout = 16,                        // operation timed out
    k_EResultBanned = 17,                        // VAC2 banned
    k_EResultAccountNotFound = 18,                // account not found
    k_EResultInvalidSteamID = 19,                // steamID is invalid
    k_EResultServiceUnavailable = 20,            // The requested service is currently unavailable
    k_EResultNotLoggedOn = 21,                    // The user is not logged on
    k_EResultPending = 22,                        // Request is pending (may be in process, or waiting on third party)
    k_EResultEncryptionFailure = 23,            // Encryption or Decryption failed
    k_EResultInsufficientPrivilege = 24,        // Insufficient privilege
    k_EResultLimitExceeded = 25,                // Too much of a good thing
    k_EResultRevoked = 26,                        // Access has been revoked (used for revoked guest passes)
    k_EResultExpired = 27,                        // License/Guest pass the user is trying to access is expired
    k_EResultAlreadyRedeemed = 28,                // Guest pass has already been redeemed by account, cannot be acked again
    k_EResultDuplicateRequest = 29,                // The request is a duplicate and the action has already occurred in the past, ignored this time
    k_EResultAlreadyOwned = 30,                    // All the games in this guest pass redemption request are already owned by the user
    k_EResultIPNotFound = 31,                    // IP address not found
    k_EResultPersistFailed = 32,                // failed to write change to the data store
    k_EResultLockingFailed = 33,                // failed to acquire access lock for this operation
    k_EResultLogonSessionReplaced = 34,
    k_EResultConnectFailed = 35,
    k_EResultHandshakeFailed = 36,
    k_EResultIOFailure = 37,
    k_EResultRemoteDisconnect = 38,
    k_EResultShoppingCartNotFound = 39,            // failed to find the shopping cart requested
    k_EResultBlocked = 40,                        // a user didn't allow it
    k_EResultIgnored = 41,                        // target is ignoring sender
    k_EResultNoMatch = 42,                        // nothing matching the request found
    k_EResultAccountDisabled = 43,
    k_EResultServiceReadOnly = 44,                // this service is not accepting content changes right now
    k_EResultAccountNotFeatured = 45,            // account doesn't have value, so this feature isn't available
    k_EResultAdministratorOK = 46,                // allowed to take this action, but only because requester is admin
    k_EResultContentVersion = 47,                // A Version mismatch in content transmitted within the Steam protocol.
    k_EResultTryAnotherCM = 48,                    // The current CM can't service the user making a request, user should try another.
    k_EResultPasswordRequiredToKickSession = 49,// You are already logged in elsewhere, this cached credential login has failed.
    k_EResultAlreadyLoggedInElsewhere = 50,        // You are already logged in elsewhere, you must wait
    k_EResultSuspended = 51,                    // Long running operation (content download) suspended/paused
    k_EResultCancelled = 52,                    // Operation canceled (typically by user: content download)
    k_EResultDataCorruption = 53,                // Operation canceled because data is ill formed or unrecoverable
    k_EResultDiskFull = 54,                        // Operation canceled - not enough disk space.
    k_EResultRemoteCallFailed = 55,                // an remote call or IPC call failed
    k_EResultPasswordUnset = 56,                // Password could not be verified as it's unset server side
    k_EResultExternalAccountUnlinked = 57,        // External account (PSN, Facebook...) is not linked to a Steam account
    k_EResultPSNTicketInvalid = 58,                // PSN ticket was invalid
    k_EResultExternalAccountAlreadyLinked = 59,    // External account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first
    k_EResultRemoteFileConflict = 60,            // The sync cannot resume due to a conflict between the local and remote files
    k_EResultIllegalPassword = 61,                // The requested new password is not legal
    k_EResultSameAsPreviousValue = 62,            // new value is the same as the old one ( secret question and answer )
    k_EResultAccountLogonDenied = 63,            // account login denied due to 2nd factor authentication failure
    k_EResultCannotUseOldPassword = 64,            // The requested new password is not legal
    k_EResultInvalidLoginAuthCode = 65,            // account login denied due to auth code invalid
    k_EResultAccountLogonDeniedNoMail = 66,        // account login denied due to 2nd factor auth failure - and no mail has been sent
    k_EResultHardwareNotCapableOfIPT = 67,        //
    k_EResultIPTInitError = 68,                    //
    k_EResultParentalControlRestricted = 69,    // operation failed due to parental control restrictions for current user
    k_EResultFacebookQueryError = 70,            // Facebook query returned an error
    k_EResultExpiredLoginAuthCode = 71,            // account login denied due to auth code expired
    k_EResultIPLoginRestrictionFailed = 72,
    k_EResultAccountLockedDown = 73,
    k_EResultAccountLogonDeniedVerifiedEmailRequired = 74,
    k_EResultNoMatchingURL = 75,
    k_EResultBadResponse = 76,                    // parse failure, missing field, etc.
    k_EResultRequirePasswordReEntry = 77,        // The user cannot complete the action until they re-enter their password
    k_EResultValueOutOfRange = 78,                // the value entered is outside the acceptable range
    k_EResultUnexpectedError = 79,                // something happened that we didn't expect to ever happen
    k_EResultDisabled = 80,                        // The requested service has been configured to be unavailable
    k_EResultInvalidCEGSubmission = 81,            // The set of files submitted to the CEG server are not valid !
    k_EResultRestrictedDevice = 82,                // The device being used is not allowed to perform this action
    k_EResultRegionLocked = 83,                    // The action could not be complete because it is region restricted
    k_EResultRateLimitExceeded = 84,            // Temporary rate limit exceeded, try again later, different from k_EResultLimitExceeded which may be permanent
    k_EResultAccountLoginDeniedNeedTwoFactor = 85,    // Need two-factor code to login
    k_EResultItemDeleted = 86,                    // The thing we're trying to access has been deleted
    k_EResultAccountLoginDeniedThrottle = 87,    // login attempt failed, try to throttle response to possible attacker
    k_EResultTwoFactorCodeMismatch = 88,        // two factor code mismatch
    k_EResultTwoFactorActivationCodeMismatch = 89,    // activation code for two-factor didn't match
    k_EResultAccountAssociatedToMultiplePartners = 90,    // account has been associated with multiple partners
    k_EResultNotModified = 91,                    // data not modified
    k_EResultNoMobileDevice = 92,                // the account does not have a mobile device associated with it
    k_EResultTimeNotSynced = 93,                // the time presented is out of range or tolerance
    k_EResultSmsCodeFailed = 94,                // SMS code failure (no match, none pending, etc.)
    k_EResultAccountLimitExceeded = 95,            // Too many accounts access this resource
    k_EResultAccountActivityLimitExceeded = 96,    // Too many changes to this account
    k_EResultPhoneActivityLimitExceeded = 97,    // Too many changes to this phone
    k_EResultRefundToWallet = 98,                // Cannot refund to payment method, must use wallet
    k_EResultEmailSendFailure = 99,                // Cannot send an email
    k_EResultNotSettled = 100,                    // Can't perform operation till payment has settled
    k_EResultNeedCaptcha = 101,                    // Needs to provide a valid captcha
    k_EResultGSLTDenied = 102,                    // a game server login token owned by this token's owner has been banned
    k_EResultGSOwnerDenied = 103,                // game server owner is denied for other reason (account lock, community ban, vac ban, missing phone)
    k_EResultInvalidItemType = 104,                // the type of thing we were requested to act on is invalid
    k_EResultIPBanned = 105,                    // the ip address has been banned from taking this action
    k_EResultGSLTExpired = 106,                    // this token has expired from disuse; can be reset for use
    k_EResultInsufficientFunds = 107,            // user doesn't have enough wallet funds to complete the action
    k_EResultTooManyPending = 108,                // There are too many of this thing pending already
    k_EResultNoSiteLicensesFound = 109,            // No site licenses found
    k_EResultWGNetworkSendExceeded = 110,        // the WG couldn't send a response because we exceeded max network send size
    k_EResultAccountNotFriends = 111,            // the user is not mutually friends
    k_EResultLimitedUserAccount = 112,            // the user is limited
    k_EResultCantRemoveItem = 113,                // item can't be removed
    k_EResultAccountDeleted = 114,                // account has been deleted
    k_EResultExistingUserCancelledLicense = 115,    // A license for this already exists, but cancelled
    k_EResultCommunityCooldown = 116,            // access is denied because of a community cooldown (probably from support profile data resets)
    k_EResultNoLauncherSpecified = 117,            // No launcher was specified, but a launcher was needed to choose correct realm for operation.
    k_EResultMustAgreeToSSA = 118,                // User must agree to china SSA or global SSA before login
    k_EResultLauncherMigrated = 119,            // The specified launcher type is no longer supported; the user should be directed elsewhere
    k_EResultSteamRealmMismatch = 120,            // The user's realm does not match the realm of the requested resource
    k_EResultInvalidSignature = 121,            // signature check did not match
    k_EResultParseFailure = 122,                // Failed to parse input
    k_EResultNoVerifiedPhone = 123,                // account does not have a verified phone number
};
