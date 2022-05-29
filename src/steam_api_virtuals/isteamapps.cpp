#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

using namespace smoke_api;

VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t appID)) { // NOLINT(misc-unused-parameters)
    return steam_apps::IsDlcUnlocked(__func__, 0, appID);
}

VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t appID)) { // NOLINT(misc-unused-parameters)
    return steam_apps::IsDlcUnlocked(__func__, 0, appID);
}

VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS()) {
    return steam_apps::GetDLCCount(__func__, 0, [&]() {
        GET_ORIGINAL_VIRTUAL_FUNCTION(ISteamApps_GetDLCCount)

        return ISteamApps_GetDLCCount_o(ARGS());
    });
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
    return steam_apps::GetDLCDataByIndex(__func__, 0, iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize, [&]() {
        GET_ORIGINAL_VIRTUAL_FUNCTION(ISteamApps_BGetDLCDataByIndex)

        return ISteamApps_BGetDLCDataByIndex_o(
            ARGS(iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize)
        );
    });
}
