#include <steam_api_virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_apps.hpp>

VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t appID)) {
    return steam_apps::IsDlcUnlocked(
        __func__, 0, appID, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(ISteamApps_BIsSubscribedApp)

            return ISteamApps_BIsSubscribedApp_o(ARGS(appID));
        }
    );
}

VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t appID)) {
    return steam_apps::IsDlcUnlocked(
        __func__, 0, appID, [&]() {
            GET_ORIGINAL_FUNCTION_STEAMAPI(ISteamApps_BIsDlcInstalled)

            return ISteamApps_BIsDlcInstalled_o(ARGS(appID));
        }
    );
}

VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS()) {
    GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_GetDLCCount)

    return steam_apps::GetDLCCount(
        __func__, 0, [&]() {
            return ISteamApps_GetDLCCount_o(ARGS());
        }
    );
}

VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(
    PARAMS(
        int iDLC,
        AppId_t* pAppID,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize
    )
) {
    return steam_apps::GetDLCDataByIndex(
        __func__, 0, iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize,
        [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamApps_BGetDLCDataByIndex)

            return ISteamApps_BGetDLCDataByIndex_o(
                ARGS(iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize)
            );
        },
        [&](AppId_t dlc_id) {
            return ISteamApps_BIsDlcInstalled(ARGS(dlc_id));
        }
    );
}
