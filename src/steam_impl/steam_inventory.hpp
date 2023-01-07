#include <core/steam_types.hpp>
#include <koalabox/types.hpp>

namespace steam_inventory {

    EResult GetResultStatus(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        const Function<EResult()>& original_function
    );

    bool GetResultItems(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize,
        const Function<bool()>& original_function,
        const Function<bool(SteamItemDef_t*, uint32_t*)>& get_item_definition_ids
    );

    bool GetResultItemProperty(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        const uint32_t* punValueBufferSizeOut,
        const Function<bool()>& original_function
    );

    bool GetAllItems(
        const String& function_name,
        const SteamInventoryResult_t* pResultHandle,
        const Function<bool()>& original_function
    );

    bool GetItemsByID(
        const String& function_name,
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        uint32_t unCountInstanceIDs,
        const Function<bool()>& original_function
    );

    bool SerializeResult(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t* punOutBufferSize,
        const Function<bool()>& original_function
    );

    bool GetItemDefinitionIDs(
        const String& function_name,
        const SteamItemDef_t* pItemDefIDs,
        uint32_t* punItemDefIDsArraySize,
        const Function<bool()>& original_function
    );


    bool CheckResultSteamID(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected,
        const Function<bool()>& original_function
    );
}
