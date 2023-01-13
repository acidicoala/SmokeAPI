#pragma once

#include <core/types.hpp>

// IClientAppManager
VIRTUAL(bool) IClientAppManager_IsAppDlcInstalled(PARAMS(AppId_t, AppId_t));

// IClientApps
VIRTUAL(int) IClientApps_GetDLCCount(PARAMS(AppId_t));
VIRTUAL(bool) IClientApps_BGetDLCDataByIndex(PARAMS(AppId_t, int, AppId_t*, bool*, char*, int));

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

// IClientUser
VIRTUAL(bool) IClientUser_BIsSubscribedApp(PARAMS(AppId_t));

// IClientUtils
VIRTUAL(AppId_t) IClientUtils_GetAppID(PARAMS());

namespace store::steamclient {

    /// We need this interface in other IClient* functions in order to call other functions
    extern Map<String, void*> interface_name_to_address_map;

    void process_client_engine(uintptr_t interface);

}
