#include <regex>

#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win_util.hpp>

#include "smoke_api/globals.hpp"
#include "smoke_api/types.hpp"
#include "steam_interface/steam_client.hpp"

namespace {
    /**
     * Searches the `.rdata` section of the original dll for the full interface version string
     * Results are cached for performance.
     */
    std::string get_versioned_interface(
        const std::string& version_prefix,
        const std::string& fallback
    ) {
        static std::map<std::string, std::string> version_map;

        if(not
            version_map.contains(version_prefix)
        )
        {
            try {
                const std::string rdata = koalabox::win_util::get_pe_section_data_or_throw(
                    globals::steamapi_module,
                    ".rdata"
                );

                const std::regex regex(version_prefix + "\\d{3}");
                std::smatch match;
                if(std::regex_search(rdata, match, regex)) {
                    version_map[version_prefix] = match[0];
                    return version_map[version_prefix];
                }

                throw koalabox::util::exception("No match found for '{}'", version_prefix);
            } catch(const std::exception& ex) {
                LOG_ERROR(
                    "Failed to get versioned interface: {}."
                    "Falling back to version {}",
                    ex.what(),
                    fallback
                );

                version_map[version_prefix] = version_prefix + fallback;
            }
        }

        return version_map[version_prefix];
    }
}

DLL_EXPORT(void*) SteamClient
(
)
 {
    static auto version = get_versioned_interface(STEAM_CLIENT, "006");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        [&] {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamClient)

            return SteamClient_o();
        }
    );
}

DLL_EXPORT(void*) SteamApps
(
)
 {
    static auto version = get_versioned_interface(STEAM_APPS, "002");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamApps)

            return SteamApps_o();
        }
    );
}

DLL_EXPORT(void*) SteamUser
(
)
 {
    static auto version = get_versioned_interface(STEAM_USER, "012");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        [&] {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamUser)

            return SteamUser_o();
        }
    );
}

DLL_EXPORT(void*) SteamInventory
(
)
 {
    static auto version = get_versioned_interface(STEAM_INVENTORY, "001");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        [&] {
            GET_ORIGINAL_FUNCTION_STEAMAPI(SteamInventory)

            return SteamInventory_o();
        }
    );
}