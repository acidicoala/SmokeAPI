#include <koalabox/logger.hpp>

#include "steam_api/exports/steam_api.hpp"
#include "smoke_api.hpp"
#include "smoke_api/config.hpp"

DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(const AppId_t unOwnAppID) {
    LOG_INFO("{} -> unOwnAppID: {}", __func__, unOwnAppID);

    // Restart can be suppressed if needed

    AUTO_CALL(SteamAPI_RestartAppIfNecessary, unOwnAppID);
}

DLL_EXPORT(void) SteamAPI_Shutdown() {
    LOG_INFO("{} -> Game requested shutdown", __func__);

    AUTO_CALL(SteamAPI_Shutdown);
}
