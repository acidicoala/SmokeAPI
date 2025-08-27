#include <ranges>
#include <set>

#include <battery/embed.hpp>

#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/win.hpp>

#include "steam_api/steam_interface.hpp"
#include "smoke_api.hpp"
#include "virtuals/steam_api_virtuals.hpp"

namespace {
    struct interface_entry {
        // function_name must match the function identifier to be able to call original functions
        std::string function_name; // e.g. "ISteamClient_GetISteamApps"
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
            {
                STEAM_APPS,
                interface_data{
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
                interface_data{
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
                interface_data{
                    .fallback_version = "SteamGameServer015",
                    .entry_map = {
                        ENTRY(ISteamGameServer, UserHasLicenseForApp),
                    }
                }
            },
            {
                STEAM_HTTP,
                interface_data{
                    .fallback_version = "STEAMHTTP_INTERFACE_VERSION003",
                    .entry_map = {
                        ENTRY(ISteamHTTP, GetHTTPResponseBodyData),
                        ENTRY(ISteamHTTP, GetHTTPStreamingResponseBodyData),
                        ENTRY(ISteamHTTP, SetHTTPRequestRawPostBody),
                    }
                }
            },
            {
                STEAM_USER,
                interface_data{
                    .fallback_version = "SteamUser023",
                    .entry_map = {
                        ENTRY(ISteamUser, UserHasLicenseForApp),
                    }
                }
            },
        };
    }

    auto read_interface_lookup() {
        std::map<std::string, std::map<std::string, uint16_t>> lookup_map;

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

namespace steam_interface {
    namespace kb = koalabox;

    AppId_t get_app_id_or_throw() {
        const auto app_id_str = kb::win::get_env_var("SteamAppId");
        return std::stoi(app_id_str);
    }

    AppId_t get_app_id() {
        try {
            static const auto app_id = get_app_id_or_throw();
            return app_id;
        } catch(const std::exception& e) {
            LOG_ERROR("Failed to get app id: {}", e.what());
            return 0;
        }
    }

    void hook_virtuals(void* interface, const std::string& version_string) {
        if(interface == nullptr) {
            // Game has tried to use an interface before initializing steam api
            return;
        }

        static std::mutex section;
        const std::lock_guard guard(section);

        static std::set<void*> processed_interfaces;

        if(processed_interfaces.contains(interface)) {
            LOG_DEBUG(
                "Interface '{}' @ {} has already been processed.",
                version_string,
                interface
            );
            return;
        }
        processed_interfaces.insert(interface);

        static const auto virtual_hook_map = get_virtual_hook_map();
        for(const auto& [prefix, data] : virtual_hook_map) {
            if(version_string.starts_with(prefix)) {
                const auto& lookup = find_lookup(version_string, data.fallback_version);

                for(const auto& [function, entry] : data.entry_map) {
                    if(lookup.contains(function)) {
                        kb::hook::swap_virtual_func(
                            interface,
                            entry.function_name,
                            lookup.at(function),
                            entry.function_address
                        );
                    }
                }

                break;
            }
        }
    }
}
