#pragma once

#include <core/types.hpp>

// Flat
DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(void*, AppId_t);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(void*, AppId_t);
DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(void*);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(void*, int, AppId_t*, bool*, char*, int);

DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(void*, HSteamUser, HSteamPipe, const char*);

DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(void*, SteamInventoryResult_t);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(void*, SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    void*, SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(void*, SteamInventoryResult_t, CSteamID);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(void*, SteamInventoryResult_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    void*, SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(void*, SteamInventoryResult_t, void*, uint32_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(void*, SteamItemDef_t*, uint32_t*);

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(void*, CSteamID, AppId_t);

// Internal
DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(HSteamUser, const char*);
DLL_EXPORT(void*) SteamInternal_CreateInterface(const char*);

// Unversioned
DLL_EXPORT(void*) CreateInterface(const char*, int*);
DLL_EXPORT(void*) SteamApps();
DLL_EXPORT(void*) SteamClient();
DLL_EXPORT(void*) SteamUser();
