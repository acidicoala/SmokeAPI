#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/hook.hpp>

using namespace smoke_api;


DLL_EXPORT(void) Log_Interface(const char* interface_name, const char* function_name) {
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

        if (compound_name == "IClientAppManager::IsAppDlcInstalled") {
            HOOK(IClientAppManager_IsAppDlcInstalled, 8)
        } else if (compound_name == "IClientApps::GetDLCCount") {
            HOOK(IClientApps_GetDLCCount, 8)
        } else if (compound_name == "IClientApps::BGetDLCDataByIndex") {
            HOOK(IClientApps_BGetDLCDataByIndex, 9)
        } else if (compound_name == "IClientUser::IsSubscribedApp") {
            HOOK(IClientUser_IsSubscribedApp, 0xB5)
        } else if (util::strings_are_equal(interface_name, "IClientInventory")) {
            if (util::strings_are_equal(function_name, "GetResultItems")) {
                auto* function_address = interface_address[0x2]; // TODO: Un-hardcode
                logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
            }
        } else if (util::strings_are_equal(interface_name, "IClientUtils")) {
            if (util::strings_are_equal(function_name, "GetAppID")) {
                auto* function_address = interface_address[0x12]; // TODO: Un-hardcode
                logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
            }
        }

        GET_ORIGINAL_FUNCTION(Log_Interface)
        Log_Interface_o(interface_name, function_name);
    } catch (const Exception& ex) {
        logger->error("{} -> Error: {}", __func__, ex.what());
    }
}
