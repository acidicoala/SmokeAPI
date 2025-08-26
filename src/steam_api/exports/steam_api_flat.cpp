#include <koalabox/logger.hpp>

#include "smoke_api.hpp"
#include "smoke_api/interfaces/steam_apps.hpp"
#include "smoke_api/interfaces/steam_inventory.hpp"
#include "smoke_api/interfaces/steam_user.hpp"
#include "steam_api/steam_client.hpp"
#include "steam_api/steam_interface.hpp"

// ISteamApps

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(void* self, const AppId_t dlcID) {
    try {
        return smoke_api::steam_apps::IsDlcUnlocked(
            __func__,
            steam_interface::get_app_id(),
            dlcID,
            MODULE_CALL_CLOSURE(SteamAPI_ISteamApps_BIsSubscribedApp, self, dlcID)
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return false;
    }
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(void* self, const AppId_t dlcID) {
    try {
        return smoke_api::steam_apps::IsDlcUnlocked(
            __func__,
            steam_interface::get_app_id(),
            dlcID,
            MODULE_CALL_CLOSURE(SteamAPI_ISteamApps_BIsDlcInstalled, self, dlcID)
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return false;
    }
}

DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(void* self) {
    try {
        return smoke_api::steam_apps::GetDLCCount(
            __func__,
            steam_interface::get_app_id(),
            MODULE_CALL_CLOSURE(SteamAPI_ISteamApps_GetDLCCount, self)
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return 0;
    }
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(
    void* self,
    const int iDLC,
    AppId_t* pDlcID,
    bool* pbAvailable,
    char* pchName,
    const int cchNameBufferSize
) {
    try {
        return smoke_api::steam_apps::GetDLCDataByIndex(
            __func__,
            steam_interface::get_app_id(),
            iDLC,
            pDlcID,
            pbAvailable,
            pchName,
            cchNameBufferSize,
            MODULE_CALL_CLOSURE(
                SteamAPI_ISteamApps_BGetDLCDataByIndex,
                self,
                iDLC,
                pDlcID,
                pbAvailable,
                pchName,
                cchNameBufferSize
            ),
            [&](const AppId_t dlc_id) {
                return SteamAPI_ISteamApps_BIsDlcInstalled(self, dlc_id);
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return false;
    }
}

// ISteamClient

DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(
    void* self,
    const HSteamUser hSteamUser,
    const HSteamPipe hSteamPipe,
    const char* pchVersion
) {
    try {
        return steam_client::GetGenericInterface(
            __func__,
            pchVersion,
            MODULE_CALL_CLOSURE(
                SteamAPI_ISteamClient_GetISteamGenericInterface,
                self,
                hSteamUser,
                hSteamPipe,
                pchVersion
            )
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

// ISteamInventory

DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(
    void* self,
    const SteamInventoryResult_t resultHandle
) {
    return smoke_api::steam_inventory::GetResultStatus(
        __func__,
        resultHandle,
        MODULE_CALL_CLOSURE(SteamAPI_ISteamInventory_GetResultStatus, self, resultHandle)
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(
    void* self,
    SteamItemDef_t* pItemDefIDs,
    uint32_t* punItemDefIDsArraySize
) {
    return smoke_api::steam_inventory::GetItemDefinitionIDs(
        __func__,
        pItemDefIDs,
        punItemDefIDsArraySize,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_GetItemDefinitionIDs,
            self,
            pItemDefIDs,
            punItemDefIDsArraySize
        )
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(
    void* self,
    const SteamInventoryResult_t resultHandle,
    SteamItemDetails_t* pOutItemsArray,
    uint32_t* punOutItemsArraySize
) {
    return smoke_api::steam_inventory::GetResultItems(
        __func__,
        resultHandle,
        pOutItemsArray,
        punOutItemsArraySize,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_GetResultItems,
            self,
            resultHandle,
            pOutItemsArray,
            punOutItemsArraySize
        ),
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            MODULE_CALL(
                SteamAPI_ISteamInventory_GetItemDefinitionIDs,
                self,
                pItemDefIDs,
                punItemDefIDsArraySize
            );
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    void* self,
    const SteamInventoryResult_t resultHandle,
    const uint32_t unItemIndex,
    const char* pchPropertyName,
    char* pchValueBuffer,
    uint32_t* punValueBufferSizeOut
) {
    return smoke_api::steam_inventory::GetResultItemProperty(
        __func__,
        resultHandle,
        unItemIndex,
        pchPropertyName,
        pchValueBuffer,
        punValueBufferSizeOut,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_GetResultItemProperty,
            self,
            resultHandle,
            unItemIndex,
            pchPropertyName,
            pchValueBuffer,
            punValueBufferSizeOut
        )
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(
    void* self,
    const SteamInventoryResult_t resultHandle,
    const CSteamID steamIDExpected
) {
    return smoke_api::steam_inventory::CheckResultSteamID(
        __func__,
        resultHandle,
        steamIDExpected,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_CheckResultSteamID,
            self,
            resultHandle,
            steamIDExpected
        )
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(
    void* self,
    SteamInventoryResult_t* pResultHandle
) {
    return smoke_api::steam_inventory::GetAllItems(
        __func__,
        pResultHandle,
        MODULE_CALL_CLOSURE(SteamAPI_ISteamInventory_GetAllItems, self, pResultHandle)
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    void* self,
    SteamInventoryResult_t* pResultHandle,
    const SteamItemInstanceID_t* pInstanceIDs,
    const uint32_t unCountInstanceIDs
) {
    return smoke_api::steam_inventory::GetItemsByID(
        __func__,
        pResultHandle,
        pInstanceIDs,
        unCountInstanceIDs,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_GetItemsByID,
            self,
            pResultHandle,
            pInstanceIDs,
            unCountInstanceIDs
        )
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(
    void* self,
    const SteamInventoryResult_t resultHandle,
    void* pOutBuffer,
    uint32_t* punOutBufferSize
) {
    return smoke_api::steam_inventory::SerializeResult(
        __func__,
        resultHandle,
        pOutBuffer,
        punOutBufferSize,
        MODULE_CALL_CLOSURE(
            SteamAPI_ISteamInventory_SerializeResult,
            self,
            resultHandle,
            pOutBuffer,
            punOutBufferSize
        )
    );
}

// ISteamUser

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(
    void* self,
    const CSteamID steamID,
    const AppId_t dlcID
) {
    try {
        return smoke_api::steam_user::UserHasLicenseForApp(
            __func__,
            steam_interface::get_app_id(),
            dlcID,
            MODULE_CALL_CLOSURE(
                SteamAPI_ISteamUser_UserHasLicenseForApp,
                self,
                steamID,
                dlcID
            )
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
}

// ISteamGameServer

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamGameServer_UserHasLicenseForApp(
    void* self,
    const CSteamID steamID,
    const AppId_t dlcID
) {
    try {
        return smoke_api::steam_user::UserHasLicenseForApp(
            __func__,
            steam_interface::get_app_id(),
            dlcID,
            MODULE_CALL_CLOSURE(
                SteamAPI_ISteamGameServer_UserHasLicenseForApp,
                self,
                steamID,
                dlcID
            )
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
}
