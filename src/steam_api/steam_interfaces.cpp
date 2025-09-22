#include <set>

#include <battery/embed.hpp>

#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include "koalabox/lib.hpp"

#include "smoke_api/steamclient/steamclient.hpp"

#include "steam_api/steam_interfaces.hpp"
#include "smoke_api/smoke_api.hpp"
#include "virtuals/steam_api_virtuals.hpp"

namespace {
    struct interface_entry {
        // function_name must match the function identifier to be able to call original functions
        std::string function_name; // e.g. "ISteamClient_GetISteamApps"
        void* function_address; // e.g. ISteamClient_GetISteamApps
    };

    struct interface_data_t { // NOLINT(*-exception-escape)
        std::string fallback_version; // e.g. "SteamClient021"
        // Key is function name without interface prefix
        std::map<std::string, interface_entry> entry_map;
        // e.g. {ENTRY(ISteamClient, GetISteamApps), ...}
    };

    // Key is interface name, e.g. "SteamClient"
    std::map<std::string, interface_data_t> get_virtual_hook_map() {
#define ENTRY(INTERFACE, FUNC) \
            { \
                #FUNC, { \
                    #INTERFACE "_" #FUNC, reinterpret_cast<void*>(INTERFACE##_##FUNC) \
                } \
            }

        return {
            {
                STEAM_APPS,
                interface_data_t{
                    .fallback_version = "STEAMAPPS_INTERFACE_VERSION008",
                    .entry_map = {
                        ENTRY(ISteamApps, BIsSubscribedApp),
                        ENTRY(ISteamApps, BIsDlcInstalled),
                        ENTRY(ISteamApps, GetDLCCount),
                        ENTRY(ISteamApps, BGetDLCDataByIndex),
                    }
                }
            },
            {
                STEAM_CLIENT,
                interface_data_t{
                    .fallback_version = "SteamClient021",
                    .entry_map = {
                        ENTRY(ISteamClient, GetISteamApps),
                        ENTRY(ISteamClient, GetISteamUser),
                        ENTRY(ISteamClient, GetISteamGenericInterface),
                        ENTRY(ISteamClient, GetISteamInventory),
                    }
                }
            },
            {
                STEAM_GAME_SERVER,
                interface_data_t{
                    .fallback_version = "SteamGameServer015",
                    .entry_map = {
                        ENTRY(ISteamGameServer, UserHasLicenseForApp),
                    }
                }
            },
            {
                STEAM_HTTP,
                interface_data_t{
                    .fallback_version = "STEAMHTTP_INTERFACE_VERSION003",
                    .entry_map = {
                        ENTRY(ISteamHTTP, GetHTTPResponseBodyData),
                        ENTRY(ISteamHTTP, GetHTTPStreamingResponseBodyData),
                        ENTRY(ISteamHTTP, SetHTTPRequestRawPostBody),
                    }
                }
            },
            {
                STEAM_INVENTORY,
                interface_data_t{
                    .fallback_version = "STEAMINVENTORY_INTERFACE_V003",
                    .entry_map = {
                        ENTRY(ISteamInventory, GetResultStatus),
                        ENTRY(ISteamInventory, GetResultItems),
                        ENTRY(ISteamInventory, CheckResultSteamID),
                        ENTRY(ISteamInventory, GetAllItems),
                        ENTRY(ISteamInventory, GetItemsByID),
                        ENTRY(ISteamInventory, SerializeResult),
                        ENTRY(ISteamInventory, GetItemDefinitionIDs),
                    }
                }
            },
            {
                STEAM_USER,
                interface_data_t{
                    .fallback_version = "SteamUser023",
                    .entry_map = {
                        ENTRY(ISteamUser, UserHasLicenseForApp),
                    }
                }
            },
            // Hooking SteamUtils for GetAppID should be avoided, since it leads to crashes in TW:WH3.
            // No idea why...
        };
    }

    // Key is function name, Value is ordinal
    using ordinal_map_t = std::map<std::string, uint16_t>;

    // Key is interface version string
    using lookup_map_t = std::map<std::string, ordinal_map_t>;

    lookup_map_t read_interface_lookup() {
        lookup_map_t lookup_map;

        const auto lookup_str = b::embed<"res/interface_lookup.json">().str();
        const auto lookup_json = nlohmann::json::parse(lookup_str);
        lookup_json.get_to(lookup_map);

        return lookup_map;
    }

    const std::map<std::string, uint16_t>& find_lookup(
        const std::string& interface_version,
        const std::string& fallback_version
    ) {
        static const auto lookup = read_interface_lookup();

        if(lookup.contains(interface_version)) {
            return lookup.at(interface_version);
        }

        LOG_WARN(
            "Interface version '{}' not found in lookup map. Using fallback: '{}'",
            interface_version,
            fallback_version
        );

        return lookup.at(fallback_version);
    }
}

namespace steam_interfaces {
    namespace kb = koalabox;

    /**
     * @param interface_ptr Pointer to the interface
     * @param version_string Example: 'SteamClient007'
     */
    void hook_virtuals(const void* interface_ptr, const std::string& version_string) {
        if(interface_ptr == nullptr) {
            // Game has tried to use an interface before initializing steam api
            // This does happen in practice.
            return;
        }

        static std::mutex section;
        const std::lock_guard guard(section);

        static std::set<const void*> processed_interfaces;

        if(processed_interfaces.contains(interface_ptr)) {
            LOG_DEBUG("Interface '{}' @ {} has already been processed.", version_string, interface_ptr);
            return;
        }
        processed_interfaces.insert(interface_ptr);

        static const auto virtual_hook_map = get_virtual_hook_map();
        for(const auto& [prefix, data] : virtual_hook_map) {
            if(not version_string.starts_with(prefix)) {
                continue;
            }

            LOG_INFO("Processing '{}' @ {} found in virtual hook map", version_string, interface_ptr);

            const auto& lookup = find_lookup(version_string, data.fallback_version);

            for(const auto& [function, entry] : data.entry_map) {
                if(not lookup.contains(function)) {
                    continue;
                }

                kb::hook::swap_virtual_func(
                    interface_ptr,
                    entry.function_name,
                    lookup.at(function),
                    entry.function_address
                );
            }

            break;
        }
    }

    void hook_steamclient_interface(
        void* steamclient_handle,
        const std::string& steam_client_interface_version
    ) noexcept {
        try {
            // Create a copy for modification
            auto virtual_hook_map = get_virtual_hook_map();

            // Remove steam client map since we don't want to hook its methods
            virtual_hook_map.erase(STEAM_CLIENT);

            // Map virtual hook map to a set of keys
            const auto prefixes = std::views::keys(virtual_hook_map) | std::ranges::to<std::set>();

            const auto CreateInterface$ = KB_LIB_GET_FUNC(steamclient_handle, CreateInterface);

            DECLARE_ARGS();
            THIS = CreateInterface$(steam_client_interface_version.c_str(), nullptr);
            hook_virtuals(THIS, steam_client_interface_version);

            const auto interface_lookup = read_interface_lookup();
            for(const auto& interface_version : interface_lookup | std::views::keys) {
                // SteamUser and SteamPipe handles must match the ones previously used by the game,
                // otherwise SteamAPI will just create new instances of interfaces, instead of returning
                // existing instances that are used by the game. Usually these handles default to 1,
                // but if a game creates several of them, then we need to somehow find them out dynamically.
                constexpr auto steam_pipe = 1;
                constexpr auto steam_user = 1;

                bool should_hook = false;
                for(const auto& prefix : prefixes) {
                    if(interface_version.starts_with(prefix)) {
                        should_hook = true;
                        break;
                    }
                }

                if(not should_hook) {
                    continue;
                }

                const auto* const interface_ptr = ISteamClient_GetISteamGenericInterface(
                    ARGS(steam_user, steam_pipe, interface_version.c_str())
                );

                if(not interface_ptr) {
                    LOG_ERROR("Failed to get generic interface: '{}'", interface_version)
                }
            }
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Unhandled exception: {}", __func__, e.what());
        }
    }

    void* find_function(
        const void* instance_ptr,
        const std::string& interface_name,
        const std::string& function_name
    ) {
        if(!get_interface_name_to_version_map().contains(interface_name)) {
            LOG_ERROR("Unsupported interface name: '{}'", interface_name);
            return nullptr;
        }
        const auto& interface_version = get_interface_name_to_version_map().at(interface_name);

        static const auto lookup = read_interface_lookup();

        if(!lookup.contains(interface_version)) {
            LOG_ERROR("Interface '{}' not found in lookup map", interface_version);
            return nullptr;
        }

        const auto interface_lookup = lookup.at(interface_version);

        if(!interface_lookup.contains(function_name)) {
            LOG_ERROR("Function '{}' not found in the map of '{}'", function_name, interface_version);
            return nullptr;
        }

        const auto ordinal = interface_lookup.at(function_name);

        const auto virtual_class = static_cast<const kb::hook::virtual_class_t*>(instance_ptr);
        return virtual_class->vtable[ordinal];
    }

    const std::map<std::string, std::string>& get_interface_name_to_version_map() {
        // Choose minimal supported versions for maximum compatibility
        // Is it better to get the interface version found in steam_api library?
        static const std::map<std::string, std::string> map = {
            {"ISteamClient", "SteamClient007"},
            {"ISteamUtils", "SteamUtils002"},
        };

        return map;
    }
}
