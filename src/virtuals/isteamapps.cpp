#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>

#include "steam_interface/steam_apps.hpp"
#include "steam_interface/steam_interface.hpp"
#include "virtuals/steam_api_virtuals.hpp"

VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t dlc_id)) {
    try {
        static const auto app_id = steam_interface::get_app_id();
        return steam_apps::IsDlcUnlocked(
            __func__,
            app_id,
            dlc_id,
            [&] {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_BIsSubscribedApp)
                return ISteamApps_BIsSubscribedApp_o(ARGS(dlc_id));
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());

        return false;
    }
}

VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t dlc_id)) {
    try {
        static const auto app_id = steam_interface::get_app_id();
        return steam_apps::IsDlcUnlocked(
            __func__,
            app_id,
            dlc_id,
            [&] {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_BIsDlcInstalled)
                return ISteamApps_BIsDlcInstalled_o(ARGS(dlc_id));
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());

        return false;
    }
}

VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS()) {
    try {
        static const auto app_id = steam_interface::get_app_id();
        return steam_apps::GetDLCCount(
            __func__,
            app_id,
            [&] {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_GetDLCCount)
                return ISteamApps_GetDLCCount_o(ARGS());
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());

        return 0;
    }
}

VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(
    PARAMS(
        int iDLC,
        AppId_t* p_dlc_id,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize
    )
) {
    try {
        static const auto app_id = steam_interface::get_app_id();
        return steam_apps::GetDLCDataByIndex(
            __func__,
            app_id,
            iDLC,
            p_dlc_id,
            pbAvailable,
            pchName,
            cchNameBufferSize,
            [&] {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_BGetDLCDataByIndex)
                return ISteamApps_BGetDLCDataByIndex_o(
                    ARGS(iDLC, p_dlc_id, pbAvailable, pchName, cchNameBufferSize)
                );
            },
            [&](AppId_t dlc_id) {
                return ISteamApps_BIsDlcInstalled(ARGS(dlc_id));
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return false;
    }
}
