#pragma once

#include <core/macros.hpp>
#include <core/types.hpp>

// TODO: Refactor into multiple headers

// ISteamClient
VIRTUAL(void*) ISteamClient_GetISteamApps(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamUser(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamInventory(PARAMS(HSteamUser, HSteamPipe, const char*));

// ISteamApps
VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t));
VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t));
VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS());
VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(PARAMS(int, AppId_t*, bool*, char*, int));

// ISteamUser
VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID, AppId_t));

// ISteamInventory
VIRTUAL(EResult) ISteamInventory_GetResultStatus(PARAMS(SteamInventoryResult_t));
VIRTUAL(bool) ISteamInventory_GetResultItems(PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*));
VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*)
);
VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t*));
VIRTUAL(bool) ISteamInventory_GetItemsByID(PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t));
VIRTUAL(bool) ISteamInventory_SerializeResult(PARAMS(SteamInventoryResult_t, void*, uint32_t*));
VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t*));
VIRTUAL(bool) ISteamInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID));

// API

DLL_EXPORT(void*) CreateInterface(const char*, int*);
DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(HSteamUser, const char*);
DLL_EXPORT(void*) SteamInternal_CreateInterface(const char*);
DLL_EXPORT(void*) SteamApps();
DLL_EXPORT(void*) SteamClient();
DLL_EXPORT(void*) SteamUser();

// Flat interfaces

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps*, AppId_t);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps*, AppId_t);
DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(ISteamApps*);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(ISteamApps*, int, AppId_t*, bool*, char*, int);
DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(ISteamUser*, CSteamID, AppId_t);
DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(ISteamClient*, HSteamUser, HSteamPipe, const char*);
DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(ISteamInventory*, SteamInventoryResult_t);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(
    ISteamInventory*, SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    ISteamInventory*, SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(ISteamInventory*, SteamInventoryResult_t, CSteamID);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(ISteamInventory*, SteamInventoryResult_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    ISteamInventory*, SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(ISteamInventory*, SteamInventoryResult_t, void*, uint32_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(ISteamInventory*, SteamItemDef_t*, uint32_t*);

// IClientApps
VIRTUAL(int) IClientApps_GetDLCCount(PARAMS(AppId_t));
VIRTUAL(bool) IClientApps_BGetDLCDataByIndex(PARAMS(AppId_t, int, AppId_t*, bool*, char*, int));

// IClientAppManager
VIRTUAL(bool) IClientAppManager_IsAppDlcInstalled(PARAMS(AppId_t, AppId_t));

// IClientUser
VIRTUAL(bool) IClientUser_BIsSubscribedApp(PARAMS(AppId_t));

// IClientInventory
VIRTUAL(EResult) IClientInventory_GetResultStatus(PARAMS(SteamInventoryResult_t));
VIRTUAL(bool) IClientInventory_GetResultItems(
    PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t, uint32_t *)
);
VIRTUAL(bool) IClientInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t, uint32_t*)
);
VIRTUAL(bool) IClientInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID));
VIRTUAL(bool) IClientInventory_GetAllItems(PARAMS(SteamInventoryResult_t*));
VIRTUAL(bool) IClientInventory_GetItemsByID(PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t));
VIRTUAL(bool) IClientInventory_SerializeResult(PARAMS(SteamInventoryResult_t, void*, uint32_t, uint32_t *));
VIRTUAL(bool) IClientInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t, uint32_t *));

namespace steam_functions {
    using namespace koalabox;

    const String STEAM_APPS = "STEAMAPPS_INTERFACE_VERSION"; // NOLINT(cert-err58-cpp)
    const String STEAM_CLIENT = "SteamClient"; // NOLINT(cert-err58-cpp)
    const String STEAM_USER = "SteamUser"; // NOLINT(cert-err58-cpp)
    const String STEAM_INVENTORY = "STEAMINVENTORY_INTERFACE_V"; // NOLINT(cert-err58-cpp)
    const String CLIENT_ENGINE = "CLIENTENGINE_INTERFACE_VERSION"; // NOLINT(cert-err58-cpp)

    void hook_virtuals(void* interface, const String& version_string);
    uint32_t get_app_id_or_throw();
}
