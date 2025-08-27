#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_apps.hpp"
#include "steam_api/steam_interface.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(const AppId_t dlc_id)) noexcept {
    return smoke_api::steam_apps::IsDlcUnlocked(
        __func__,
        steam_interface::get_app_id(),
        dlc_id,
        HOOKED_CALL_CLOSURE(ISteamApps_BIsSubscribedApp, ARGS(dlc_id))
    );
}

VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(const AppId_t dlc_id)) noexcept {
    return smoke_api::steam_apps::IsDlcUnlocked(
        __func__,
        steam_interface::get_app_id(),
        dlc_id,
        HOOKED_CALL_CLOSURE(ISteamApps_BIsDlcInstalled, ARGS(dlc_id))
    );
}

VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS()) noexcept {
    return smoke_api::steam_apps::GetDLCCount(
        __func__,
        steam_interface::get_app_id(),
        HOOKED_CALL_CLOSURE(ISteamApps_GetDLCCount, ARGS())
    );
}

VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(
    PARAMS(
        const int iDLC,
        AppId_t* p_dlc_id,
        bool* pbAvailable,
        char* pchName,
        const int cchNameBufferSize
    )
) noexcept {
    return smoke_api::steam_apps::GetDLCDataByIndex(
        __func__,
        steam_interface::get_app_id(),
        iDLC,
        p_dlc_id,
        pbAvailable,
        pchName,
        cchNameBufferSize,
        HOOKED_CALL_CLOSURE(
            ISteamApps_BGetDLCDataByIndex,
            ARGS(iDLC, p_dlc_id, pbAvailable, pchName, cchNameBufferSize)
        ),
        HOOKED_CALL_CLOSURE(
           ISteamApps_BIsSubscribedApp,
           ARGS(*p_dlc_id)
       )
    );
}
