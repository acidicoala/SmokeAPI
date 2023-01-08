#pragma once

#include <koalabox/core.hpp>

#ifdef _WIN64
#define COMPILE_KOALAGEDDON 0
#else
#define COMPILE_KOALAGEDDON 1
#endif

using AppId_t = uint32_t;
using SteamInventoryResult_t = uint32_t;
using SteamItemInstanceID_t = uint64_t;
using SteamItemDef_t = uint32_t;
using HSteamPipe = uint32_t;
using HSteamUser = uint32_t;
using CSteamID = uint64_t;
using EResult = uint32_t;

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

class ISteamClient;

class ISteamApps;

class ISteamUser;

class ISteamInventory;
