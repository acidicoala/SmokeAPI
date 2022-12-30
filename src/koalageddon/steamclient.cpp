#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/hook.hpp>

using namespace smoke_api;

void hook_function(
    const void* interface_address,
    const FunctionAddress& hook_function,
    const String& name,
    const int ordinal
) {
    static Set<String> hooked_functions;

    if (interface_address == nullptr) {
        return;
    }

    if (hooked_functions.contains(name)) {
        return;
    }

    hook::swap_virtual_func_or_throw(interface_address, name, ordinal, hook_function);

    hooked_functions.insert(name);
}

DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name) {
    try {
        void**** parent_ebp;
        __asm mov parent_ebp, ebp

        const auto offset_one = koalageddon_config.steamclient_interceptor_function_address_offset_one;
        auto offset_two = koalageddon_config.steamclient_interceptor_function_address_offset_two;

        const auto compound_name = interface_name + String("::") + function_name;

//        logger->trace("Intercepted interface function: '{}'", compound_name);

#define HOOK_INTERFACE(FUNC) hook_function( \
    parent_ebp[offset_one][offset_two],     \
    (FunctionAddress) FUNC,                 \
    #FUNC,                                  \
    koalageddon_config.FUNC##_ordinal       \
);

        if (util::strings_are_equal(interface_name, "IClientAppManager")) {
            HOOK_INTERFACE(IClientAppManager_IsAppDlcInstalled)
            // TODO: Investigate IClientAppManager::BIsDlcEnabled
            // TODO: Investigate IClientAppManager::GetDlcSizes
            // TODO: Investigate IClientAppManager::GetInstalledApps
        } else if (util::strings_are_equal(interface_name, "IClientApps")) {
            HOOK_INTERFACE(IClientApps_GetDLCCount)
            HOOK_INTERFACE(IClientApps_BGetDLCDataByIndex)
            // TODO: Investigate IClientApps::GetAppKVRaw
            // TODO: Investigate IClientApps::GetAppDataSection
            // TODO: Investigate IClientApps::GetAppData
        } else if (util::strings_are_equal(interface_name, "IClientInventory")) {
            HOOK_INTERFACE(IClientInventory_GetResultStatus)
            HOOK_INTERFACE(IClientInventory_GetResultItems)
            HOOK_INTERFACE(IClientInventory_GetResultItemProperty)
            HOOK_INTERFACE(IClientInventory_CheckResultSteamID)
            HOOK_INTERFACE(IClientInventory_GetAllItems)
            HOOK_INTERFACE(IClientInventory_GetItemsByID)
            HOOK_INTERFACE(IClientInventory_SerializeResult)
            HOOK_INTERFACE(IClientInventory_GetItemDefinitionIDs)
        } else if (util::strings_are_equal(interface_name, "IClientUser")) {
            // NOTE: parent_ebp[offset_one][offset_two] will always be 0 for IClientUser interface.
            // Probably because it actually represents a handle with 0 being the default user.
            // However, the real interface is still accessible at a neighboring offset.
            offset_two = koalageddon_config.steamclient_interceptor_function_address_offset_two_client_user;

            HOOK_INTERFACE(IClientUser_IsSubscribedApp)
            // TODO: Investigate IClientUser::GetConfigString
            // TODO: Investigate IClientUser::GetConfigStoreKeyName
            // TODO: Investigate IClientUser::GetSubscribedApps
            // TODO: Investigate IClientUser::GetUserConfigFolder
            // TODO: Investigate IClientUser::GetAppIDForGameID
        }

        // TODO: Investigate IClientDeviceAuth::GetSharedLibraryLockedBy?

        GET_ORIGINAL_FUNCTION(SteamClient_Interface_Interceptor)
        SteamClient_Interface_Interceptor_o(interface_name, function_name);
    } catch (const Exception& ex) {
        logger->error("{} -> Error: {}", __func__, ex.what());
    }
}
