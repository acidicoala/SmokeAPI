#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/hook.hpp>

using namespace smoke_api;

DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name) {
    try {
        void**** parent_ebp;

        __asm mov parent_ebp, ebp

        auto* interface_address = (*parent_ebp)[2];

        static Set<String> hooked_functions;

        auto hook_function = [&](const auto hook_function, const String& name, const int ordinal) {
            if (hooked_functions.contains(name)) {
                return;
            }

            hook::swap_virtual_func_or_throw(interface_address, name, ordinal, (FunctionAddress) (hook_function));

            hooked_functions.insert(name);
        };

        const auto compound_name = interface_name + String("::") + function_name;

#define HOOK(FUNC, ORDINAL) hook_function(FUNC, #FUNC, ORDINAL);
        // TODO: Parametrize ordinals
        if (util::strings_are_equal(interface_name, "IClientAppManager")) {
            HOOK(IClientAppManager_IsAppDlcInstalled, 8)
        } else if (util::strings_are_equal(interface_name, "IClientApps")) {
            HOOK(IClientApps_GetDLCCount, 8)
            HOOK(IClientApps_BGetDLCDataByIndex, 9)
        } else if (util::strings_are_equal(interface_name, "IClientInventory")) {
            HOOK(IClientInventory_GetResultStatus, 0)
            HOOK(IClientInventory_GetResultItems, 2)
            HOOK(IClientInventory_GetResultItemProperty, 3)
            HOOK(IClientInventory_CheckResultSteamID, 5)
            HOOK(IClientInventory_GetAllItems, 8)
            HOOK(IClientInventory_GetItemsByID, 9)
            HOOK(IClientInventory_SerializeResult, 6)
            HOOK(IClientInventory_GetItemDefinitionIDs, 19)
        }

        GET_ORIGINAL_FUNCTION(SteamClient_Interface_Interceptor)
        SteamClient_Interface_Interceptor_o(interface_name, function_name);
    } catch (const Exception& ex) {
        logger->error("{} -> Error: {}", __func__, ex.what());
    }
}
