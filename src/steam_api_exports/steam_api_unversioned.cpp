#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/win_util.hpp>

#include <regex>

using namespace smoke_api;
using namespace steam_functions;

/**
 * Searches the `.rdata` section of the original dll for the full interface version string
 * Results are cached for performance.
 */
String get_versioned_interface(const String& version_prefix, const String& fallback) {
    static Map<String, String> version_map;

    if (not version_map.contains(version_prefix)) {
        try {
            String rdata = win_util::get_pe_section_data_or_throw(original_library, ".rdata");

            std::regex regex(version_prefix + "\\d{3}");
            std::smatch match;
            if (std::regex_search(rdata, match, regex)) {
                version_map[version_prefix] = match[0];
                return version_map[version_prefix];
            }

            throw util::exception("No match found for '{}'", version_prefix);
        } catch (const Exception& ex) {
            logger->error(
                "Failed to get versioned interface: {}."
                "Falling back to version {}", ex.what(), fallback
            );

            version_map[version_prefix] = version_prefix + fallback;
        }
    }

    return version_map[version_prefix];
}

DLL_EXPORT(void*) SteamClient() {
    static auto version = get_versioned_interface(STEAM_CLIENT, "006");

    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamClient)

        return SteamClient_o();
    });
}

DLL_EXPORT(void*) SteamApps() {
    static auto version = get_versioned_interface(STEAM_APPS, "002");

    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamApps)

        return SteamApps_o();
    });
}

DLL_EXPORT(void*) SteamUser() {
    static auto version = get_versioned_interface(STEAM_USER, "012");

    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamUser)

        return SteamUser_o();
    });
}


DLL_EXPORT(void*) SteamInventory() {
    static auto version = get_versioned_interface(STEAM_INVENTORY, "001");

    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamInventory)

        return SteamInventory_o();
    });
}
