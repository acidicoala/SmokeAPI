#include <map>
#include <regex>

#include <koalabox/logger.hpp>
#include <koalabox/win.hpp>

#include "smoke_api/smoke_api.hpp"
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

        if(version_map.contains(version_prefix)) {
            return version_map.at(version_prefix);
        }

        try {
            const auto section = kb::win::get_pe_section_or_throw(
                smoke_api::steamapi_module,
                ".rdata"
            );
            const auto rdata = std::string(
                reinterpret_cast<const char*>(section.start_address),
                section.size
            );

            const std::regex regex(version_prefix + "\\d{3}");
            if(std::smatch match; std::regex_search(rdata, match, regex)) {
                version_map[version_prefix] = match[0];
            } else {
                throw std::runtime_error(std::format("No match found for '{}'", version_prefix));
            }
        } catch(const std::exception& ex) {
            LOG_ERROR(
                "Failed to get versioned interface: {}."
                "Falling back to version {}",
                ex.what(),
                fallback
            );

            version_map[version_prefix] = version_prefix + fallback;
        }

        return version_map.at(version_prefix);
    }
}

// TODO: Do we really need to proxy them?

#define MODULE_CALL_CLOSURE(FUNC, ...) \
    [&] { \
        static const auto _##FUNC = KB_WIN_GET_PROC(smoke_api::steamapi_module, FUNC); \
        return _##FUNC(__VA_ARGS__); \
    }

DLL_EXPORT(void*) SteamApps() {
    static auto version = get_versioned_interface(STEAM_APPS, "002");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamApps)
    );
}

DLL_EXPORT(void*) SteamClient() {
    static auto version = get_versioned_interface(STEAM_CLIENT, "006");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamClient)
    );
}

DLL_EXPORT(void*) SteamHTTP() {
    static auto version = get_versioned_interface(STEAM_HTTP, "003");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamHTTP)
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

DLL_EXPORT(void*) SteamUser() {
    static auto version = get_versioned_interface(STEAM_USER, "012");

    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamUser)
    );
}
