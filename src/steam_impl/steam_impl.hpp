#include <steam_functions/steam_functions.hpp>

using namespace koalabox;

namespace steam_apps {

    bool IsSubscribedApp(const String& function_name, AppId_t appID);

    bool IsDlcInstalled(const String& function_name, AppId_t appID);

    int GetDLCCount(const String& function_name, const std::function<int()>& original_function);

    bool GetDLCDataByIndex(
        const String& function_name,
        int iDLC,
        AppId_t* pAppID,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const std::function<bool()>& original_function
    );

}

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const std::function<EUserHasLicenseForAppResult()>& original_function
    );

}

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const std::function<void*()>& original_function
    );

}

namespace steam_inventory {

    EResult GetResultStatus(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        const std::function<EResult()>& original_function
    );

    bool GetResultItems(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize,
        const std::function<bool()>& original_function,
        const std::function<bool(SteamItemDef_t*, uint32_t*)>& get_item_definition_ids
    );

    bool GetResultItemProperty(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        const uint32_t* punValueBufferSizeOut,
        const std::function<bool()>& original_function
    );

    bool GetAllItems(
        const String& function_name,
        const SteamInventoryResult_t* pResultHandle,
        const std::function<bool()>& original_function
    );

    bool GetItemsByID(
        const String& function_name,
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        uint32_t unCountInstanceIDs,
        const std::function<bool()>& original_function
    );

    bool SerializeResult(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t* punOutBufferSize,
        const std::function<bool()>& original_function
    );

    bool GetItemDefinitionIDs(
        const String& function_name,
        const SteamItemDef_t* pItemDefIDs,
        uint32_t* punItemDefIDsArraySize,
        const std::function<bool()>& original_function
    );

    bool GetItemDefinitionProperty(
        const String& function_name,
        SteamItemDef_t iDefinition,
        const char* pchPropertyName,
        char* pchValueBuffer,
        const uint32_t* punValueBufferSizeOut,
        const std::function<bool()>& original_function
    );

    bool CheckResultSteamID(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected,
        const std::function<bool()>& original_function
    );
}
