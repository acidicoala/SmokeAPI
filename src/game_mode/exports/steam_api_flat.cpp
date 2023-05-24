#include <steam_impl/steam_apps.hpp>
#include <steam_impl/steam_client.hpp>
#include <steam_impl/steam_inventory.hpp>
#include <steam_impl/steam_user.hpp>
#include <steam_impl/steam_game_server.hpp>
#include <steam_impl/steam_impl.hpp>
#include <koalabox/logger.hpp>

// ISteamApps

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(void* self, AppId_t dlcID) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_apps::IsDlcUnlocked(
            __func__, app_id, dlcID, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamApps_BIsSubscribedApp)

                return SteamAPI_ISteamApps_BIsSubscribedApp_o(self, dlcID);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return false;
    }
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(void* self, AppId_t dlcID) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_apps::IsDlcUnlocked(
            __func__, app_id, dlcID, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamApps_BIsDlcInstalled)

                return SteamAPI_ISteamApps_BIsDlcInstalled_o(self, dlcID);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return false;
    }
}

DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(void* self) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_apps::GetDLCCount(
            __func__, app_id, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamApps_GetDLCCount)

                return SteamAPI_ISteamApps_GetDLCCount_o(self);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return 0;
    }
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(
    void* self,
    int iDLC,
    AppId_t* pDlcID,
    bool* pbAvailable,
    char* pchName,
    int cchNameBufferSize
) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_apps::GetDLCDataByIndex(
            __func__, app_id, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize,
            [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamApps_BGetDLCDataByIndex)
                return SteamAPI_ISteamApps_BGetDLCDataByIndex_o(
                    self, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize
                );
            },
            [&](AppId_t dlc_id) {
                return SteamAPI_ISteamApps_BIsDlcInstalled(self, dlc_id);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return false;
    }
}

// ISteamClient

DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(
    void* self,
    HSteamUser hSteamUser,
    HSteamPipe hSteamPipe,
    const char* pchVersion
) {
    try {
        return steam_client::GetGenericInterface(
            __func__, pchVersion, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamClient_GetISteamGenericInterface)
                return SteamAPI_ISteamClient_GetISteamGenericInterface_o(self, hSteamUser, hSteamPipe, pchVersion);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return nullptr;
    }
}

// ISteamInventory

DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(
    void* self,
    SteamInventoryResult_t resultHandle
) {
    return steam_inventory::GetResultStatus(
        __func__, resultHandle, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetResultStatus)

            return SteamAPI_ISteamInventory_GetResultStatus_o(self, resultHandle);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(
    void* self,
    SteamItemDef_t* pItemDefIDs,
    uint32_t* punItemDefIDsArraySize
) {
    return steam_inventory::GetItemDefinitionIDs(
        __func__, pItemDefIDs, punItemDefIDsArraySize, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetItemDefinitionIDs)

            return SteamAPI_ISteamInventory_GetItemDefinitionIDs_o(self, pItemDefIDs, punItemDefIDsArraySize);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(
    void* self,
    SteamInventoryResult_t resultHandle,
    SteamItemDetails_t* pOutItemsArray,
    uint32_t* punOutItemsArraySize
) {
    return steam_inventory::GetResultItems(
        __func__, resultHandle, pOutItemsArray, punOutItemsArraySize,
        [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetResultItems)

            return SteamAPI_ISteamInventory_GetResultItems_o(self, resultHandle, pOutItemsArray, punOutItemsArraySize);
        },
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetItemDefinitionIDs)

            return SteamAPI_ISteamInventory_GetItemDefinitionIDs_o(self, pItemDefIDs, punItemDefIDsArraySize);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    void* self,
    SteamInventoryResult_t resultHandle,
    uint32_t unItemIndex,
    const char* pchPropertyName,
    char* pchValueBuffer,
    uint32_t* punValueBufferSizeOut
) {
    return steam_inventory::GetResultItemProperty(
        __func__, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetResultItemProperty)

            return SteamAPI_ISteamInventory_GetResultItemProperty_o(
                self, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut
            );
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(
    void* self,
    SteamInventoryResult_t resultHandle,
    CSteamID steamIDExpected
) {
    return steam_inventory::CheckResultSteamID(
        __func__, resultHandle, steamIDExpected, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_CheckResultSteamID)

            return SteamAPI_ISteamInventory_CheckResultSteamID_o(self, resultHandle, steamIDExpected);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(
    void* self,
    SteamInventoryResult_t* pResultHandle
) {
    return steam_inventory::GetAllItems(
        __func__, pResultHandle, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetAllItems)

            return SteamAPI_ISteamInventory_GetAllItems_o(self, pResultHandle);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    void* self,
    SteamInventoryResult_t* pResultHandle,
    const SteamItemInstanceID_t* pInstanceIDs,
    uint32_t unCountInstanceIDs
) {
    return steam_inventory::GetItemsByID(
        __func__, pResultHandle, pInstanceIDs, unCountInstanceIDs, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_GetItemsByID)

            return SteamAPI_ISteamInventory_GetItemsByID_o(self, pResultHandle, pInstanceIDs, unCountInstanceIDs);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(
    void* self,
    SteamInventoryResult_t resultHandle,
    void* pOutBuffer,
    uint32_t* punOutBufferSize
) {
    return steam_inventory::SerializeResult(
        __func__, resultHandle, pOutBuffer, punOutBufferSize, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamInventory_SerializeResult)

            return SteamAPI_ISteamInventory_SerializeResult_o(self, resultHandle, pOutBuffer, punOutBufferSize);
        }
    );
}

// ISteamUser

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(
    void* self,
    CSteamID steamID,
    AppId_t dlcID
) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_user::UserHasLicenseForApp(
            __func__, app_id, dlcID, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamUser_UserHasLicenseForApp)

                return SteamAPI_ISteamUser_UserHasLicenseForApp_o(self, steamID, dlcID);
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }

}

// ISteamGameServer

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamGameServer_UserHasLicenseForApp(
    void* self,
    CSteamID steamID,
    AppId_t dlcID
) {
    try {
        /*
        Will crash here.
        Probably because server only app don't need to init same interfaces used to recover the appID in the client (ISteamUser).
        */
        //static const auto app_id = steam_impl::get_app_id_or_throw();
        // TODO : CLEAN THIS BY FINDING A WAY TO GET AppID FROM ISteamGameServer (hooking InitGameServer maybe ?)
        static const auto app_id = 0; // Workaround
        return steam_game_server::UserHasLicenseForApp(
            __func__, app_id, dlcID, [&]() {
                GET_ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_ISteamGameServer_UserHasLicenseForApp)

                    return SteamAPI_ISteamGameServer_UserHasLicenseForApp_o(self, steamID, dlcID);
            }
        );
    }
    catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
            return k_EUserHasLicenseResultDoesNotHaveLicense;
    }

}