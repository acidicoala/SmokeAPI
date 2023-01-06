#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_inventory.hpp>

using namespace smoke_api;

VIRTUAL(EResult) IClientInventory_GetResultStatus(PARAMS(SteamInventoryResult_t resultHandle)) {
    return steam_inventory::GetResultStatus(__func__, resultHandle, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetResultStatus)

        return IClientInventory_GetResultStatus_o(ARGS(resultHandle));
    });
}

VIRTUAL(bool) IClientInventory_GetResultItems(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray, // 1st pass: null, 2nd pass: ptr to out array
        uint32_t item_count, // 1st pass: 0, 2nd pass: array size
        uint32_t * punOutItemsArraySize // 1st pass: ptr to out array size, 2nd pass: ptr to 0
    )
) {
    return steam_inventory::GetResultItems(
        __func__, resultHandle, pOutItemsArray, punOutItemsArraySize,
        [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetResultItems)

            *punOutItemsArraySize = item_count;
            return IClientInventory_GetResultItems_o(
                ARGS(resultHandle, pOutItemsArray, item_count, punOutItemsArraySize)
            );
        },
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetItemDefinitionIDs)

            return IClientInventory_GetItemDefinitionIDs_o(
                ARGS(pItemDefIDs, *punItemDefIDsArraySize, punItemDefIDsArraySize)
            );
        }
    );
}

VIRTUAL(bool) IClientInventory_GetResultItemProperty(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        uint32_t item_count,
        uint32_t * punValueBufferSizeOut
    )
) {
    return steam_inventory::GetResultItemProperty(
        __func__, resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetResultItemProperty)

            *punValueBufferSizeOut = item_count;
            return IClientInventory_GetResultItemProperty_o(
                ARGS(resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, item_count, punValueBufferSizeOut)
            );
        }
    );
}

VIRTUAL(bool) IClientInventory_CheckResultSteamID(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected
    )
) {
    return steam_inventory::CheckResultSteamID(__func__, resultHandle, steamIDExpected, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_CheckResultSteamID)

        return IClientInventory_CheckResultSteamID_o(ARGS(resultHandle, steamIDExpected));
    });
}

VIRTUAL(bool) IClientInventory_GetAllItems(PARAMS(SteamInventoryResult_t* pResultHandle)) {
    return steam_inventory::GetAllItems(__func__, pResultHandle, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetAllItems)

        return IClientInventory_GetAllItems_o(ARGS(pResultHandle));
    });
}

VIRTUAL(bool) IClientInventory_GetItemsByID(
    PARAMS(
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        uint32_t unCountInstanceIDs
    )
) {
    return steam_inventory::GetItemsByID(__func__, pResultHandle, pInstanceIDs, unCountInstanceIDs, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetItemsByID)

        return IClientInventory_GetItemsByID_o(ARGS(pResultHandle, pInstanceIDs, unCountInstanceIDs));
    });
}

VIRTUAL(bool) IClientInventory_SerializeResult(
    PARAMS(
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t buffer_size,
        uint32_t * punOutBufferSize
    )
) {
    return steam_inventory::SerializeResult(__func__, resultHandle, pOutBuffer, punOutBufferSize, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_SerializeResult)

        *punOutBufferSize = buffer_size;
        return IClientInventory_SerializeResult_o(ARGS(resultHandle, pOutBuffer, buffer_size, punOutBufferSize));
    });
}

VIRTUAL(bool) IClientInventory_GetItemDefinitionIDs(
    PARAMS(
        SteamItemDef_t* pItemDefIDs, // 1st pass: null, 2nd pass: ptr to out array
        uint32_t item_count, // 1st pass: 0, 2nd pass: array size
        uint32_t * p_array_size  // 1st pass: ptr to out array size, 2nd pass: ptr to 0
    )
) {
    return steam_inventory::GetItemDefinitionIDs(__func__, pItemDefIDs, p_array_size, [&]() {
        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_GetItemDefinitionIDs)

        *p_array_size = item_count;
        return IClientInventory_GetItemDefinitionIDs_o(ARGS(pItemDefIDs, item_count, p_array_size));
    });
}
