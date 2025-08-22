#include <ranges>
#include <set>

#include <battery/embed.hpp>
#include <polyhook2/Misc.hpp>

#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win_util.hpp>

#include "build_config.h"

#include "steam_interface/steam_interface.hpp"
#include "virtuals/steam_api_virtuals.hpp"

namespace {

    struct interface_entry {
        // function_name must match the function identifier to be able to call original functions
        std::string function_name;  // e.g. "ISteamClient_GetISteamApps"
        uintptr_t function_address; // e.g. ISteamClient_GetISteamApps
    };

    struct interface_data {
        std::string fallback_version; // e.g. "SteamClient021"
        std::map<std::string, interface_entry> entry_map;
        // e.g. {ENTRY(ISteamClient, GetISteamApps), ...}
    };

    std::map<std::string, interface_data> get_virtual_hook_map() {

#define ENTRY(INTERFACE, FUNC)                                                                     \
    {                                                                                              \
        #FUNC, {                                                                                   \
            #INTERFACE "_" #FUNC, reinterpret_cast<uintptr_t>(INTERFACE##_##FUNC)                  \
        }                                                                                          \
    }

        return {
            {STEAM_CLIENT,
             interface_data{
                 .fallback_version = "SteamClient021",
                 .entry_map =
                     {
                         ENTRY(ISteamClient, GetISteamApps),
                         ENTRY(ISteamClient, GetISteamUser),
                         ENTRY(ISteamClient, GetISteamGenericInterface),
                         ENTRY(ISteamClient, GetISteamInventory),
                     }
             }},
            {STEAM_APPS,
             interface_data{
                 .fallback_version = "STEAMAPPS_INTERFACE_VERSION008",
                 .entry_map =
                     {
                         ENTRY(ISteamApps, BIsSubscribedApp),
                         ENTRY(ISteamApps, BIsDlcInstalled),
                         ENTRY(ISteamApps, GetDLCCount),
                         ENTRY(ISteamApps, BGetDLCDataByIndex),
                     }
             }},
            {STEAM_USER,
             interface_data{
                 .fallback_version = "SteamUser023",
                 .entry_map =
                     {
                         ENTRY(ISteamUser, UserHasLicenseForApp),
                     }
             }},
            {STEAM_INVENTORY,
             interface_data{
                 .fallback_version = "STEAMINVENTORY_INTERFACE_V003",
                 .entry_map =
                     {
                         ENTRY(ISteamInventory, GetResultStatus),
                         ENTRY(ISteamInventory, GetResultItems),
                         ENTRY(ISteamInventory, CheckResultSteamID),
                         ENTRY(ISteamInventory, GetAllItems),
                         ENTRY(ISteamInventory, GetItemsByID),
                         ENTRY(ISteamInventory, SerializeResult),
                         ENTRY(ISteamInventory, GetItemDefinitionIDs),
                     }
             }},
        };
    }

    nlohmann::json read_interface_lookup() {
        const auto lookup_str = b::embed<"res/interface_lookup.json">().str();
        return nlohmann::json::parse(lookup_str);
    }

    const nlohmann::json& find_lookup(
        const std::string& interface_version, //
        const std::string& fallback_version
    ) {
        static const auto lookup = read_interface_lookup();

        if (lookup.contains(interface_version)) {
            return lookup[interface_version];
        }

        LOG_WARN(
            "Interface version '{}' not found in lookup map. Using fallback: '{}'",
            interface_version,
            fallback_version
        );

        return lookup[fallback_version];
    }
}

namespace steam_interface {
    namespace kb = koalabox;

    void hook_virtuals(void* interface, const std::string& version_string) {
        if (interface == nullptr) {
            // Game has tried to use an interface before initializing steam api
            return;
        }

        static std::set<void*> processed_interfaces;

        if (processed_interfaces.contains(interface)) {
            LOG_DEBUG("Interface {} at {} has already been processed.", version_string, interface);
            return;
        }

        static std::mutex section;
        const std::lock_guard guard(section);

        static const auto virtual_hook_map = get_virtual_hook_map();
        for (const auto& [prefix, data] : virtual_hook_map) {
            if (version_string.starts_with(prefix)) {
                const auto& lookup = find_lookup(version_string, data.fallback_version);

                for (const auto& [function, entry] : data.entry_map) {
                    if (lookup.contains(function)) {
                        kb::hook::swap_virtual_func(
                            interface, entry.function_name, lookup[function], entry.function_address
                        );
                    }
                }

                break;
            }
        }

        processed_interfaces.insert(interface);
    }

    HSteamPipe get_steam_pipe_or_throw() {
        // TODO: Is there a more elegant way of getting steam pipe?

        const auto& steam_api_module = koalabox::win_util::get_module_handle_or_throw(STEAMAPI_DLL);
        void* GetHSteamPipe_address;
        try {
            GetHSteamPipe_address = (void*)koalabox::win_util::get_proc_address_or_throw(
                steam_api_module, "SteamAPI_GetHSteamPipe"
            );
        } catch (...) {
            GetHSteamPipe_address = (void*)koalabox::win_util::get_proc_address_or_throw(
                steam_api_module, "GetHSteamPipe"
            );
        }
        typedef HSteamPipe(__cdecl * GetHSteamPipe_t)();
        const auto GetHSteamPipe_o = (GetHSteamPipe_t)GetHSteamPipe_address;
        return GetHSteamPipe_o();
    }

    template <typename F> F get_virtual_function(void* interface, int ordinal) {
        auto* v_table = (void***)interface;
        return (F)(*v_table)[ordinal];
    }

    template <typename F, typename... Args>
    auto call_virtual_function(void* interface, F function, Args... args) {
#ifdef _WIN64
        void* RCX = interface;
#else
        void* ECX = interface;
        void* EDX = nullptr;
#endif

        return function(ARGS(args...));
    }

    // TODO: Test if this actually works!!!
    AppId_t get_app_id_or_throw() {
        TCHAR buffer[32];
        GetEnvironmentVariable(TEXT("SteamAppId"), buffer, sizeof(buffer));

        return std::stoi(buffer);
    }

    AppId_t get_app_id() {
        try {
            return get_app_id_or_throw();
        } catch (...) {
            LOG_ERROR("Failed to get app id");
            return 0;
        }

        /*// Get CreateInterface
        const auto& steam_client_module =
            koalabox::win_util::get_module_handle_or_throw(STEAMCLIENT_DLL);
        auto* CreateInterface_address = (void*)koalabox::win_util::get_proc_address_or_throw(
            steam_client_module, "CreateInterface"
        );
        auto* CreateInterface_o = PLH::FnCast(CreateInterface_address, CreateInterface);

        // Get ISteamClient
        int result;
        auto* i_steam_client = CreateInterface_o("SteamClient006", &result);
        if (i_steam_client == nullptr) {
            throw koalabox::util::exception(
                "Failed to obtain SteamClient006 interface. Result: {}", result
            );
        }

        // Get GetISteamUtils
        typedef void***(__fastcall * GetISteamUtils_t)(
            PARAMS(HSteamPipe hSteamPipe, const char* version)
        );
        const auto steam_utils_ordinal = steam_client_ordinal_map["ISteamClient_GetISteamUtils"][6];
        const auto GetISteamUtils_o =
            get_virtual_function<GetISteamUtils_t>(i_steam_client, steam_utils_ordinal);

        // Get ISteamUtils
        const auto steam_pipe = get_steam_pipe_or_throw();
        auto* i_steam_utils =
            call_virtual_function(i_steam_client, GetISteamUtils_o, steam_pipe, "SteamUtils002");

        // Get GetAppID
        typedef uint32_t(__fastcall * GetAppID_t)(PARAMS());
        const auto get_app_id_ordinal = steam_utils_ordinal_map["ISteamUtils_GetAppID"][2];
        const auto GetAppID_o = get_virtual_function<GetAppID_t>(i_steam_utils, get_app_id_ordinal);

        return call_virtual_function(i_steam_utils, GetAppID_o);*/
    }
}