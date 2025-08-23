#include <regex>
#include <map>

#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win_util.hpp>

#include "smoke_api.hpp"
#include "steam_api/steam_client.hpp"

namespace {
    namespace kb = koalabox;

    /**
     * Searches the `.rdata` section of the original dll for the full interface version string
     * Results are cached for performance.
     */
    std::string get_versioned_interface(
        const std::string& version_prefix,
        const std::string& fallback
    ) {
        static std::map<std::string, std::string> version_map;

        if(not version_map.contains(version_prefix)) {
            try {
                const std::string rdata = kb::win_util::get_pe_section_data_or_throw(
                    smoke_api::steamapi_module,
                    ".rdata"
                );

                const std::regex regex(version_prefix + "\\d{3}");
                if(std::smatch match; std::regex_search(rdata, match, regex)) {
                    version_map[version_prefix] = match[0];
                    return version_map[version_prefix];
                }

                throw kb::util::exception("No match found for '{}'", version_prefix);
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

DLL_EXPORT(void*) SteamClient() {
    static auto version = get_versioned_interface(STEAM_CLIENT, "006");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamClient)
    );
}

DLL_EXPORT(void*) SteamApps() {
    static auto version = get_versioned_interface(STEAM_APPS, "002");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamApps)
    );
}

DLL_EXPORT(void*) SteamUser() {
    static auto version = get_versioned_interface(STEAM_USER, "012");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamUser)
    );
}

DLL_EXPORT(void*) SteamInventory() {
    static auto version = get_versioned_interface(STEAM_INVENTORY, "001");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamInventory)
    );
}
