#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_apps.hpp>

using namespace smoke_api;

VIRTUAL(int) IClientApps_GetDLCCount(PARAMS(AppId_t appId)) {
    return steam_apps::GetDLCCount(__func__, appId, [&]() {
        GET_ORIGINAL_VIRTUAL_FUNCTION(IClientApps_GetDLCCount)

        return IClientApps_GetDLCCount_o(ARGS(appId));
    });
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
    return steam_apps::GetDLCDataByIndex(__func__, appID, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize, [&]() {
        GET_ORIGINAL_VIRTUAL_FUNCTION(IClientApps_BGetDLCDataByIndex)

        return IClientApps_BGetDLCDataByIndex_o(
            ARGS(appID, iDLC, pDlcID, pbAvailable, pchName, cchNameBufferSize)
        );
    });
}
