#include <ranges>
#include <steam_impl/steam_impl.hpp>
#include <game_mode/virtuals/steam_api_virtuals.hpp>
#include <common/steamclient_exports.hpp>
#include <build_config.h>
#include <koalabox/util.hpp>
#include <koalabox/win_util.hpp>
#include <koalabox/logger.hpp>
#include <polyhook2/Misc.hpp>

#if COMPILE_STORE_MODE
#include <store_mode/steamclient/steamclient.hpp>
#endif

namespace steam_impl {

    typedef Map<String, Map<int, int>> FunctionOrdinalMap;

    FunctionOrdinalMap steam_client_ordinal_map = {  // NOLINT(cert-err58-cpp)
        {"ISteamClient_GetISteamApps",
            {
                {6,  16},
                {7,  18},
                {8,  15},
                {9, 16},
                {12, 15},
            }
        },
        {"ISteamClient_GetISteamUser",
            {
                {6,  6},
                {7,  5},
            }
        },
        {"ISteamClient_GetISteamUtils",
            {
                {6,  12},
                {7,  9},
            }
        },
        {"ISteamClient_GetISteamGenericInterface",
            {
                {7,  14},
                {8,  13},
                {12, 12},
            }
        },
        {"ISteamClient_GetISteamInventory",
            {
                {17, 34},
                {18, 35},
            }
        }
    };

    FunctionOrdinalMap steam_apps_ordinal_map = { // NOLINT(cert-err58-cpp)
        {"ISteamApps_BIsSubscribedApp",   {{2, 6}}},
        {"ISteamApps_BIsDlcInstalled",    {{2, 7}}},
        {"ISteamApps_GetDLCCount",        {{2, 10}}},
        {"ISteamApps_BGetDLCDataByIndex", {{2, 11}}},
    };

    FunctionOrdinalMap steam_user_ordinal_map = { // NOLINT(cert-err58-cpp)
        {"ISteamUser_UserHasLicenseForApp", {
            {12, 15},
            {13, 16},
            {15, 17},
        }}
    };

    FunctionOrdinalMap steam_utils_ordinal_map = { // NOLINT(cert-err58-cpp)
        {"ISteamUtils_GetAppID", {{2, 9}}}
    };

    FunctionOrdinalMap steam_inventory_ordinal_map = { // NOLINT(cert-err58-cpp)
        {"ISteamInventory_GetResultStatus",       {{1, 0}}},
        {"ISteamInventory_GetResultItems",        {{1, 1}}},
        {"ISteamInventory_GetResultItemProperty", {{2, 2}}},
        {"ISteamInventory_CheckResultSteamID",    {{1, 3},  {2, 4}}},
        {"ISteamInventory_GetAllItems",           {{1, 5},  {2, 6}}},
        {"ISteamInventory_GetItemsByID",          {{1, 6},  {2, 7}}},
        {"ISteamInventory_SerializeResult",       {{1, 7},  {2, 8}}},
        {"ISteamInventory_GetItemDefinitionIDs",  {{1, 20}, {2, 21}}},
    };

    int extract_version_number(
        const String& version_string,
        const String& prefix,
        int min_version,
        int max_version
    ) {
        LOG_DEBUG("Hooking interface '{}'", version_string)

        try {
            const auto version_number = stoi(version_string.substr(prefix.length()));

            if (version_number < min_version) {
                LOG_WARN("Legacy version of {}: {}", version_string, version_number)
            }

            if (version_number > max_version) {
                LOG_WARN(
                    "Unsupported new version of {}: {}. Fallback version {} will be used",
                    version_string, version_number, max_version
                )
            }

            return version_number;
        } catch (const std::exception& ex) {
            koalabox::util::panic("Failed to extract version number from: '{}'", version_string);
        }
    }

    int get_ordinal(const FunctionOrdinalMap& ordinal_map, const String& function_name, int interface_version) {
        const auto& map = ordinal_map.at(function_name);

        for (auto [version, ordinal]: std::ranges::reverse_view(map)) {
            if (interface_version >= version) {
                return ordinal;
            }
        }

        koalabox::util::panic("Invalid interface version ({}) for function {}", interface_version, function_name);
    }

#define HOOK_VIRTUALS(MAP, FUNC) \
    koalabox::hook::swap_virtual_func( \
        interface,      \
        #FUNC, \
        get_ordinal(MAP, #FUNC, version_number), \
        reinterpret_cast<uintptr_t>(FUNC) \
    );

#define HOOK_STEAM_CLIENT(FUNC)     HOOK_VIRTUALS(steam_client_ordinal_map, FUNC)
#define HOOK_STEAM_APPS(FUNC)       HOOK_VIRTUALS(steam_apps_ordinal_map, FUNC)
#define HOOK_STEAM_USER(FUNC)       HOOK_VIRTUALS(steam_user_ordinal_map, FUNC)
#define HOOK_STEAM_INVENTORY(FUNC)  HOOK_VIRTUALS(steam_inventory_ordinal_map, FUNC)

    void hook_virtuals(void* interface, const String& version_string) {
        if (interface == nullptr) {
            // Game has tried to use an interface before initializing steam api
            return;
        }

        static Set<void*> hooked_interfaces;

        if (hooked_interfaces.contains(interface)) {
            // This interface is already hooked. Skipping it.
            return;
        }

        static Mutex section;
        const MutexLockGuard guard(section);

        if (version_string.starts_with(STEAM_CLIENT)) {
            const auto version_number = extract_version_number(version_string, STEAM_CLIENT, 6, 20);

            HOOK_STEAM_CLIENT(ISteamClient_GetISteamApps)
            HOOK_STEAM_CLIENT(ISteamClient_GetISteamUser)
            if (version_number >= 7) {
                HOOK_STEAM_CLIENT(ISteamClient_GetISteamGenericInterface)
            }
            if (version_number >= 17) {
                HOOK_STEAM_CLIENT(ISteamClient_GetISteamInventory)
            }
        } else if (version_string.starts_with(STEAM_APPS)) {
            const auto version_number = extract_version_number(version_string, STEAM_APPS, 2, 8);

            HOOK_STEAM_APPS(ISteamApps_BIsSubscribedApp)

            if (version_number >= 3) {
                HOOK_STEAM_APPS(ISteamApps_BIsDlcInstalled)
            }

            if (version_number >= 4) {
                HOOK_STEAM_APPS(ISteamApps_GetDLCCount)
                HOOK_STEAM_APPS(ISteamApps_BGetDLCDataByIndex)
            }
        } else if (version_string.starts_with(STEAM_USER)) {
            const auto version_number = extract_version_number(version_string, STEAM_USER, 9, 21);

            if (version_number >= 12) {
                HOOK_STEAM_USER(ISteamUser_UserHasLicenseForApp)
            }
        } else if (version_string.starts_with(STEAM_INVENTORY)) {
            const auto version_number = extract_version_number(version_string, STEAM_INVENTORY, 1, 3);

            HOOK_STEAM_INVENTORY(ISteamInventory_GetResultStatus)
            HOOK_STEAM_INVENTORY(ISteamInventory_GetResultItems)
            HOOK_STEAM_INVENTORY(ISteamInventory_CheckResultSteamID)
            HOOK_STEAM_INVENTORY(ISteamInventory_GetAllItems)
            HOOK_STEAM_INVENTORY(ISteamInventory_GetItemsByID)
            HOOK_STEAM_INVENTORY(ISteamInventory_SerializeResult)
            HOOK_STEAM_INVENTORY(ISteamInventory_GetItemDefinitionIDs)

            if (version_number >= 2) {
                HOOK_STEAM_INVENTORY(ISteamInventory_GetResultItemProperty)
            }
        } else if (version_string.starts_with(CLIENT_ENGINE)) {
#if COMPILE_STORE_MODE
            store::steamclient::process_client_engine(reinterpret_cast<uintptr_t>(interface));
#endif
        } else {
            return;
        }

        hooked_interfaces.insert(interface);
    }

    HSteamPipe get_steam_pipe_or_throw() {
        const auto& steam_api_module = koalabox::win_util::get_module_handle_or_throw(STEAMAPI_DLL);
        void* GetHSteamPipe_address;
        try {
            GetHSteamPipe_address = (void*) koalabox::win_util::get_proc_address_or_throw(
                steam_api_module, "SteamAPI_GetHSteamPipe"
            );
        } catch (const Exception& ex) {
            GetHSteamPipe_address = (void*) koalabox::win_util::get_proc_address_or_throw(
                steam_api_module, "GetHSteamPipe"
            );
        }
        typedef HSteamPipe (__cdecl* GetHSteamPipe_t)();
        const auto GetHSteamPipe_o = (GetHSteamPipe_t) GetHSteamPipe_address;
        return GetHSteamPipe_o();
    }

    template<typename F>
    F get_virtual_function(void* interface, int ordinal) {
        auto* v_table = (void***) interface;
        return (F) (*v_table)[ordinal];
    }

    template<typename F, typename... Args>
    auto call_virtual_function(void* interface, F function, Args... args) {
#ifdef _WIN64
        void* RCX = interface;
#else
        void* ECX = interface;
        void* EDX = nullptr;
#endif

        return function(ARGS(args...));
    }

    AppId_t get_app_id_or_throw() {
        // Get CreateInterface
        const auto& steam_client_module = koalabox::win_util::get_module_handle_or_throw(STEAMCLIENT_DLL);
        auto* CreateInterface_address = (void*) koalabox::win_util::get_proc_address_or_throw(
            steam_client_module, "CreateInterface"
        );
        auto* CreateInterface_o = PLH::FnCast(CreateInterface_address, CreateInterface);

        // Get ISteamClient
        int result;
        auto* i_steam_client = CreateInterface_o("SteamClient006", &result);
        if (i_steam_client == nullptr) {
            throw koalabox::util::exception("Failed to obtain SteamClient006 interface. Result: {}", result);
        }

        // Get GetISteamUtils
        typedef void*** (__fastcall* GetISteamUtils_t)(PARAMS(HSteamPipe hSteamPipe, const char* version));
        const auto steam_utils_ordinal = steam_client_ordinal_map["ISteamClient_GetISteamUtils"][6];
        const auto GetISteamUtils_o = get_virtual_function<GetISteamUtils_t>(i_steam_client, steam_utils_ordinal);

        // Get ISteamUtils
        const auto steam_pipe = get_steam_pipe_or_throw();
        auto* i_steam_utils = call_virtual_function(i_steam_client, GetISteamUtils_o, steam_pipe, "SteamUtils002");

        // Get GetAppID
        typedef uint32_t (__fastcall* GetAppID_t)(PARAMS());
        const auto get_app_id_ordinal = steam_utils_ordinal_map["ISteamUtils_GetAppID"][2];
        const auto GetAppID_o = get_virtual_function<GetAppID_t>(i_steam_utils, get_app_id_ordinal);

        return call_virtual_function(i_steam_utils, GetAppID_o);
    }
}
