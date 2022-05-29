#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/hook.hpp>

#pragma warning(disable: 4731) // 'ebp' modified by inline assembly...

using namespace smoke_api;

DLL_EXPORT(void) Log_Interface(const char* interface_name, const char* function_name) {
    void***** parent_ebp;

    __asm mov parent_ebp, ebp

    auto* interface_address = *((*parent_ebp)[2]);

    if (util::strings_are_equal(interface_name, "IClientAppManager")) {
        if (util::strings_are_equal(function_name, "IsAppDlcInstalled")) {
            auto* function_address = interface_address[0x8]; // TODO: Un-hardcode
            logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
        }
    } else if (util::strings_are_equal(interface_name, "IClientApps")) {
        if (util::strings_are_equal(function_name, "BGetDLCDataByIndex")) {
            auto* function_address = interface_address[0x9]; // TODO: Un-hardcode
            logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
        } else if (util::strings_are_equal(function_name, "GetDLCCount")) {
            auto* function_address = interface_address[0x8]; // TODO: Un-hardcode
            logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
        }
    } else if (util::strings_are_equal(interface_name, "IClientInventory")) {
        if (util::strings_are_equal(function_name, "GetResultItems")) {
            auto* function_address = interface_address[0x2]; // TODO: Un-hardcode
            logger->debug("{} -> {}::{} @ {}", __func__, interface_name, function_name, function_address);
        }
    } else if (util::strings_are_equal(interface_name, "IClientUser")) {
        if (util::strings_are_equal(function_name, "IsSubscribedApp")) {
            auto* function_address = interface_address[0xB5]; // TODO: Un-hardcode
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
}
