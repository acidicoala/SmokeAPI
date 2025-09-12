#include "smoke_api/interfaces/steam_inventory.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(EResult) ISteamInventory_GetResultStatus(
    PARAMS(const SteamInventoryResult_t resultHandle)
) noexcept {
    return smoke_api::steam_inventory::GetResultStatus(
        __func__,
        resultHandle,
        SWAPPED_CALL_CLOSURE(ISteamInventory_GetResultStatus, ARGS(resultHandle))
    );
}

VIRTUAL(bool) ISteamInventory_GetResultItems(
    PARAMS(
        const SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize
    )
)  noexcept {
    return smoke_api::steam_inventory::GetResultItems(
        __func__,
        resultHandle,
        pOutItemsArray,
        punOutItemsArraySize,
        SWAPPED_CALL_CLOSURE(
            ISteamInventory_GetResultItems,
            ARGS(resultHandle, pOutItemsArray, punOutItemsArraySize)
        ),
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            SWAPPED_CALL(
                THIS,
                ISteamInventory_GetItemDefinitionIDs,
                ARGS(pItemDefIDs, punItemDefIDsArraySize)
            );
        }
    );
}

VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(
        const SteamInventoryResult_t resultHandle,
        const uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        uint32_t* punValueBufferSizeOut
    )
) noexcept {
    return smoke_api::steam_inventory::GetResultItemProperty(
        __func__,
        resultHandle,
        unItemIndex,
        pchPropertyName,
        pchValueBuffer,
        punValueBufferSizeOut,
        SWAPPED_CALL_CLOSURE(
            ISteamInventory_GetResultItemProperty,
            ARGS(
                resultHandle,
                unItemIndex,
                pchPropertyName,
                pchValueBuffer,
                punValueBufferSizeOut
            )
        )
    );
}

VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t* pResultHandle)) noexcept {
    return smoke_api::steam_inventory::GetAllItems(
        __func__,
        pResultHandle,
        SWAPPED_CALL_CLOSURE(ISteamInventory_GetAllItems, ARGS(pResultHandle))
    );
}

VIRTUAL(bool) ISteamInventory_GetItemsByID(
    PARAMS(
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        const uint32_t unCountInstanceIDs
    )
) noexcept {
    return smoke_api::steam_inventory::GetItemsByID(
        __func__,
        pResultHandle,
        pInstanceIDs,
        unCountInstanceIDs,
        SWAPPED_CALL_CLOSURE(
            ISteamInventory_GetItemsByID,
            ARGS(pResultHandle, pInstanceIDs, unCountInstanceIDs)
        )
    );
}

VIRTUAL(bool) ISteamInventory_SerializeResult(
    PARAMS(
        const SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t* punOutBufferSize
    )
) noexcept {
    return smoke_api::steam_inventory::SerializeResult(
        __func__,
        resultHandle,
        pOutBuffer,
        punOutBufferSize,
        SWAPPED_CALL_CLOSURE(
            ISteamInventory_SerializeResult,
            ARGS(resultHandle, pOutBuffer, punOutBufferSize)
        )
    );
}

VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(
    PARAMS(
        SteamItemDef_t*pItemDefIDs,
        uint32_t* punItemDefIDsArraySize
    )
) noexcept {
    return smoke_api::steam_inventory::GetItemDefinitionIDs(
        __func__,
        pItemDefIDs,
        punItemDefIDsArraySize,
        SWAPPED_CALL_CLOSURE(
            ISteamInventory_GetItemDefinitionIDs,
            ARGS(pItemDefIDs, punItemDefIDsArraySize)
        )
    );
}

VIRTUAL(bool) ISteamInventory_CheckResultSteamID(
    PARAMS(const SteamInventoryResult_t resultHandle, CSteamID steamIDExpected)
) noexcept {
    return smoke_api::steam_inventory::CheckResultSteamID(
        __func__,
        resultHandle,
        steamIDExpected,
        SWAPPED_CALL_CLOSURE(ISteamInventory_CheckResultSteamID, ARGS(resultHandle, steamIDExpected))
    );
}
