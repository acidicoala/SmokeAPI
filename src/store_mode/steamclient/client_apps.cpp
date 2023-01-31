#include <store_mode/steamclient/steamclient.hpp>
#include <steam_impl/steam_apps.hpp>

VIRTUAL(int) IClientApps_GetDLCCount(PARAMS(AppId_t appId)) {
    try {
        return steam_apps::GetDLCCount(
            __func__, appId, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(IClientApps_GetDLCCount)

                return IClientApps_GetDLCCount_o(ARGS(appId));
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return 0;
    }
}

VIRTUAL(bool) IClientApps_BGetDLCDataByIndex(
    PARAMS(
        AppId_t appID,
        int iDLC,
        AppId_t* pDlcID,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize
    )
) {
    try {
        return steam_apps::GetDLCDataByIndex(
            __func__, appID, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize,
            [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(IClientApps_BGetDLCDataByIndex)

                return IClientApps_BGetDLCDataByIndex_o(
                    ARGS(appID, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize)
                );
            },
            [&](AppId_t dlc_id) {
                const auto* app_manager_interface = store::steamclient::interface_name_to_address_map["IClientAppManager"];
                if (app_manager_interface) {
                    IClientAppManager_IsAppDlcInstalled(app_manager_interface, EDX, appID, dlc_id);
                    return true;
                }

                // Would never happen in practice, as the interfaces would be instantiated almost simultaneously
                return false;
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return false;
    }
}
