#include "smoke_api.hpp"
#include "smoke_api/interfaces/steam_inventory.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(EResult) ISteamInventory_GetResultStatus(
    PARAMS(const SteamInventoryResult_t resultHandle)
) {
    return smoke_api::steam_inventory::GetResultStatus(
        __func__,
        resultHandle,
        HOOKED_CALL_CLOSURE(ISteamInventory_GetResultStatus, ARGS(resultHandle))
    );
}

VIRTUAL(bool) ISteamInventory_GetResultItems(
    PARAMS(
        const SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize
    )
) {
    return smoke_api::steam_inventory::GetResultItems(
        __func__,
        resultHandle,
        pOutItemsArray,
        punOutItemsArraySize,
        HOOKED_CALL_CLOSURE(
            ISteamInventory_GetResultItems,
            ARGS(resultHandle, pOutItemsArray, punOutItemsArraySize)
        ),
        [&](SteamItemDef_t* pItemDefIDs, uint32_t* punItemDefIDsArraySize) {
            HOOKED_CALL(
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
) {
    return smoke_api::steam_inventory::GetResultItemProperty(
        __func__,
        resultHandle,
        unItemIndex,
        pchPropertyName,
        pchValueBuffer,
        punValueBufferSizeOut,
        HOOKED_CALL_CLOSURE(
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

VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t* pResultHandle)) {
    return smoke_api::steam_inventory::GetAllItems(
        __func__,
        pResultHandle,
        HOOKED_CALL_CLOSURE(ISteamInventory_GetAllItems, ARGS(pResultHandle))
    );
}

VIRTUAL(bool) ISteamInventory_GetItemsByID(
    PARAMS(
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        const uint32_t unCountInstanceIDs
    )
) {
    return smoke_api::steam_inventory::GetItemsByID(
        __func__,
        pResultHandle,
        pInstanceIDs,
        unCountInstanceIDs,
        HOOKED_CALL_CLOSURE(
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
) {
    return smoke_api::steam_inventory::SerializeResult(
        __func__,
        resultHandle,
        pOutBuffer,
        punOutBufferSize,
        HOOKED_CALL_CLOSURE(
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
) {
    return smoke_api::steam_inventory::GetItemDefinitionIDs(
        __func__,
        pItemDefIDs,
        punItemDefIDsArraySize,
        HOOKED_CALL_CLOSURE(
            ISteamInventory_GetItemDefinitionIDs,
            ARGS(pItemDefIDs, punItemDefIDsArraySize)
        )
    );
}

VIRTUAL(bool) ISteamInventory_CheckResultSteamID(
    PARAMS(SteamInventoryResult_t resultHandle, CSteamID steamIDExpected)
) {
    return smoke_api::steam_inventory::CheckResultSteamID(
        __func__,
        resultHandle,
        steamIDExpected,
        HOOKED_CALL_CLOSURE(
            ISteamInventory_CheckResultSteamID,
            ARGS(resultHandle, steamIDExpected)
        )
    );
}
