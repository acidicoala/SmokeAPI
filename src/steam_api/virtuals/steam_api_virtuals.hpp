#pragma once

#include "smoke_api/types.hpp"

// ISteamApps
VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t)) noexcept;
VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t)) noexcept;
VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS()) noexcept;
VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(PARAMS(int, AppId_t*, bool*, char*, int)) noexcept;

// ISteamClient
VIRTUAL(void*) ISteamClient_GetISteamApps(PARAMS(HSteamUser, HSteamPipe, const char*)) noexcept;
VIRTUAL(void*) ISteamClient_GetISteamUser(PARAMS(HSteamUser, HSteamPipe, const char*)) noexcept;
VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(
    PARAMS(HSteamUser, HSteamPipe, const char*)

) noexcept;
VIRTUAL(void*) ISteamClient_GetISteamInventory(
    PARAMS(HSteamUser, HSteamPipe, const char*)

) noexcept;

// ISteamHTTP
VIRTUAL(bool) ISteamHTTP_GetHTTPResponseBodyData(
    PARAMS(HTTPRequestHandle, const uint8_t*, uint32_t)
) noexcept;
VIRTUAL(bool) ISteamHTTP_GetHTTPStreamingResponseBodyData(
    PARAMS(HTTPRequestHandle, uint32_t, const uint8_t*, uint32_t)
) noexcept;
VIRTUAL(bool) ISteamHTTP_SetHTTPRequestRawPostBody(
    PARAMS(HTTPRequestHandle, const char*, const uint8_t*, uint32_t)
) noexcept;

// ISteamInventory
VIRTUAL(EResult) ISteamInventory_GetResultStatus(PARAMS(SteamInventoryResult_t)) noexcept;
VIRTUAL(bool) ISteamInventory_GetResultItems(
    PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*) // @formatter:off
) noexcept; // @formatter:on

VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*) // @formatter:off
) noexcept; // @formatter:on
VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t*)) noexcept;
VIRTUAL(bool) ISteamInventory_GetItemsByID(
    PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t)
) noexcept;
VIRTUAL(bool) ISteamInventory_SerializeResult(
    PARAMS(SteamInventoryResult_t, void*, uint32_t*)

) noexcept;
VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t*)) noexcept;
VIRTUAL(bool) ISteamInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID)) noexcept;

// ISteamUser
VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(
    PARAMS(CSteamID, AppId_t)
) noexcept;

// ISteamGameServer
VIRTUAL(EUserHasLicenseForAppResult) ISteamGameServer_UserHasLicenseForApp(
    PARAMS(CSteamID, AppId_t)
) noexcept;
