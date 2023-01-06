#pragma once

typedef uint32_t SteamInventoryResult_t;
typedef uint64_t SteamItemInstanceID_t;
typedef uint32_t SteamItemDef_t;
typedef uint32_t AppId_t;
typedef uint32_t HSteamPipe;
typedef uint32_t HSteamUser;
typedef uint64_t CSteamID;

struct SteamItemDetails_t {
    SteamItemInstanceID_t m_itemId;
    uint32_t m_iDefinition;
    uint16_t m_unQuantity;
    uint16_t m_unFlags; // see ESteamItemFlags
};

// results from UserHasLicenseForApp
enum EUserHasLicenseForAppResult {
    k_EUserHasLicenseResultHasLicense = 0,         // User has a license for specified app
    k_EUserHasLicenseResultDoesNotHaveLicense = 1, // User does not have a license for the specified app
    k_EUserHasLicenseResultNoAuth = 2,             // User has not been authenticated
};

typedef uint32_t EResult;
