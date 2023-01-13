#include <game_mode/virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_inventory.hpp>

VIRTUAL(EResult) ISteamInventory_GetResultStatus(PARAMS(SteamInventoryResult_t resultHandle)) {
    return steam_inventory::GetResultStatus(
        __func__, resultHandle, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetResultStatus)

            return ISteamInventory_GetResultStatus_o(ARGS(resultHandle));
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetResultItems(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t * punOutItemsArraySize
    )
) {
    return steam_inventory::GetResultItems(
        __func__, resultHandle, pOutItemsArray, punOutItemsArraySize,
        [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetResultItems)

            return ISteamInventory_GetResultItems_o(ARGS(resultHandle, pOutItemsArray, punOutItemsArraySize));
        },
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetItemDefinitionIDs)

            return ISteamInventory_GetItemDefinitionIDs_o(ARGS(pItemDefIDs, punItemDefIDsArraySize));
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        uint32_t * punValueBufferSizeOut
    )
) {
    return steam_inventory::GetResultItemProperty(
        __func__, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetResultItemProperty)

            return ISteamInventory_GetResultItemProperty_o(
                ARGS(resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut)
            );
        }
    );
}

VIRTUAL(bool) ISteamInventory_CheckResultSteamID(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected
    )
) {
    return steam_inventory::CheckResultSteamID(
        __func__, resultHandle, steamIDExpected, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_CheckResultSteamID)

            return ISteamInventory_CheckResultSteamID_o(ARGS(resultHandle, steamIDExpected));
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t* pResultHandle)) {
    return steam_inventory::GetAllItems(
        __func__, pResultHandle, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetAllItems)

            return ISteamInventory_GetAllItems_o(ARGS(pResultHandle));
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetItemsByID(
    PARAMS(
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        uint32_t unCountInstanceIDs
    )
) {
    return steam_inventory::GetItemsByID(
        __func__, pResultHandle, pInstanceIDs, unCountInstanceIDs, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetItemsByID)

            return ISteamInventory_GetItemsByID_o(ARGS(pResultHandle, pInstanceIDs, unCountInstanceIDs));
        }
    );
}

VIRTUAL(bool) ISteamInventory_SerializeResult(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t * punOutBufferSize
    )
) {
    return steam_inventory::SerializeResult(
        __func__, resultHandle, pOutBuffer, punOutBufferSize, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_SerializeResult)

            return ISteamInventory_SerializeResult_o(ARGS(resultHandle, pOutBuffer, punOutBufferSize));
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(
    PARAMS(
        SteamItemDef_t* pItemDefIDs,
        uint32_t * punItemDefIDsArraySize
    )
) {
    return steam_inventory::GetItemDefinitionIDs(
        __func__, pItemDefIDs, punItemDefIDsArraySize, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamInventory_GetItemDefinitionIDs)

            return ISteamInventory_GetItemDefinitionIDs_o(ARGS(pItemDefIDs, punItemDefIDsArraySize));
        }
    );
}
