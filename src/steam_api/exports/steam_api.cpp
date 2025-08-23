#include <koalabox/logger.hpp>

#include "smoke_api.hpp"
#include "smoke_api/config.hpp"

// TODO: Support in hook mode
DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(const uint32_t unOwnAppID) {
    if(smoke_api::config::instance.override_app_id != 0) {
        LOG_DEBUG("{} -> {}. Preventing app restart", unOwnAppID, __func__);
        return false;
    }

    // Note: Assumes proxy mode only
    MODULE_CALL(SteamAPI_RestartAppIfNecessary, unOwnAppID);
}

// TODO: Support in hook mode
DLL_EXPORT(void) SteamAPI_Shutdown() {
    LOG_INFO("{} -> Game requested shutdown", __func__);

    // Note: Assumes proxy mode only
    MODULE_CALL(SteamAPI_Shutdown);
}
