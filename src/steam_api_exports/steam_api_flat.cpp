#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

using namespace smoke_api;

// ISteamApps

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps*, AppId_t appID) {
    return steam_apps::IsDlcUnlocked(__func__, 0, appID);
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps*, AppId_t appID) {
    return steam_apps::IsDlcUnlocked(__func__, 0, appID);
}

DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(ISteamApps* self) {
    return steam_apps::GetDLCCount(__func__, 0, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamApps_GetDLCCount)

        return SteamAPI_ISteamApps_GetDLCCount_o(self);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(
    ISteamApps* self,
    int iDLC,
    AppId_t* pAppID,
    bool* pbAvailable,
    char* pchName,
    int cchNameBufferSize
) {
    return steam_apps::GetDLCDataByIndex(__func__, 0, iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamApps_BGetDLCDataByIndex)

        return SteamAPI_ISteamApps_BGetDLCDataByIndex_o(
            self, iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize
        );
    });
}

// ISteamUser

DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(
    ISteamUser* self,
    CSteamID steamID,
    AppId_t appID
) {
    return steam_user::UserHasLicenseForApp(__func__, appID, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamUser_UserHasLicenseForApp)

        return SteamAPI_ISteamUser_UserHasLicenseForApp_o(self, steamID, appID);
    });
}

// ISteamClient

DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(
    ISteamClient* self,
    HSteamUser hSteamUser,
    HSteamPipe hSteamPipe,
    const char* pchVersion
) {
    return steam_client::GetGenericInterface(__func__, pchVersion, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamClient_GetISteamGenericInterface)

        return SteamAPI_ISteamClient_GetISteamGenericInterface_o(self, hSteamUser, hSteamPipe, pchVersion);
    });
}

// ISteamInventory

DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(
    ISteamInventory* self,
    SteamInventoryResult_t resultHandle
) {
    return steam_inventory::GetResultStatus(__func__, resultHandle, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetResultStatus)

        return SteamAPI_ISteamInventory_GetResultStatus_o(self, resultHandle);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(
    ISteamInventory* self,
    SteamInventoryResult_t resultHandle,
    SteamItemDetails_t* pOutItemsArray,
    uint32_t* punOutItemsArraySize
) {
    return steam_inventory::GetResultItems(
        __func__, resultHandle, pOutItemsArray, punOutItemsArraySize,
        [&]() {
            GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetResultItems)

            return SteamAPI_ISteamInventory_GetResultItems_o(self, resultHandle, pOutItemsArray, punOutItemsArraySize);
        },
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetItemDefinitionIDs)

            return SteamAPI_ISteamInventory_GetItemDefinitionIDs_o(self, pItemDefIDs, punItemDefIDsArraySize);
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    ISteamInventory* self,
    SteamInventoryResult_t resultHandle,
    uint32_t unItemIndex,
    const char* pchPropertyName,
    char* pchValueBuffer,
    const uint32_t* punValueBufferSizeOut
) {
    return steam_inventory::GetResultItemProperty(
        __func__, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut, [&]() {
            GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetResultItemProperty)

            return SteamAPI_ISteamInventory_GetResultItemProperty_o(
                self, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut
            );
        }
    );
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(
    ISteamInventory* self,
    SteamInventoryResult_t resultHandle,
    CSteamID steamIDExpected
) {
    return steam_inventory::CheckResultSteamID(__func__, resultHandle, steamIDExpected, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_CheckResultSteamID)

        return SteamAPI_ISteamInventory_CheckResultSteamID_o(self, resultHandle, steamIDExpected);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(
    ISteamInventory* self,
    SteamInventoryResult_t* pResultHandle
) {
    return steam_inventory::GetAllItems(__func__, pResultHandle, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetAllItems)

        return SteamAPI_ISteamInventory_GetAllItems_o(self, pResultHandle);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    ISteamInventory* self,
    SteamInventoryResult_t* pResultHandle,
    const SteamItemInstanceID_t* pInstanceIDs,
    uint32_t unCountInstanceIDs
) {
    return steam_inventory::GetItemsByID(__func__, pResultHandle, pInstanceIDs, unCountInstanceIDs, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetItemsByID)

        return SteamAPI_ISteamInventory_GetItemsByID_o(self, pResultHandle, pInstanceIDs, unCountInstanceIDs);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(
    ISteamInventory* self,
    SteamInventoryResult_t resultHandle,
    void* pOutBuffer,
    uint32_t* punOutBufferSize
) {
    return steam_inventory::SerializeResult(__func__, resultHandle, pOutBuffer, punOutBufferSize, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_SerializeResult)

        return SteamAPI_ISteamInventory_SerializeResult_o(self, resultHandle, pOutBuffer, punOutBufferSize);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(
    ISteamInventory* self,
    SteamItemDef_t* pItemDefIDs,
    uint32_t* punItemDefIDsArraySize
) {
    return steam_inventory::GetItemDefinitionIDs(__func__, pItemDefIDs, punItemDefIDsArraySize, [&]() {
        GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetItemDefinitionIDs)

        return SteamAPI_ISteamInventory_GetItemDefinitionIDs_o(self, pItemDefIDs, punItemDefIDsArraySize);
    });
}

DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionProperty(
    ISteamInventory* self,
    SteamItemDef_t iDefinition,
    const char* pchPropertyName,
    char* pchValueBuffer,
    uint32_t* punValueBufferSizeOut
) {
    return steam_inventory::GetItemDefinitionProperty(
        __func__, iDefinition, pchPropertyName, pchValueBuffer, punValueBufferSizeOut, [&]() {
            GET_ORIGINAL_FUNCTION(SteamAPI_ISteamInventory_GetItemDefinitionProperty)

            return SteamAPI_ISteamInventory_GetItemDefinitionProperty_o(
                self, iDefinition, pchPropertyName, pchValueBuffer, punValueBufferSizeOut
            );
        }
    );
}
