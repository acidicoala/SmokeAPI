#pragma once

#include <core/types.hpp>

// Flat
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

// Internal
DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(HSteamUser, const char*);
DLL_EXPORT(void*) SteamInternal_CreateInterface(const char*);

// Unversioned
DLL_EXPORT(void*) CreateInterface(const char*, int*);
DLL_EXPORT(void*) SteamApps();
DLL_EXPORT(void*) SteamClient();
DLL_EXPORT(void*) SteamUser();
