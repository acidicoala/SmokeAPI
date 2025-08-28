#include <koalabox/logger.hpp>

#include "steam_api/exports/steam_api.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/smoke_api.hpp"

#define AUTO_CALL(FUNC, ...) \
    static const auto _##FUNC = smoke_api::hook_mode \
        ? KB_HOOK_GET_HOOKED_FN(FUNC) \
        : KB_WIN_GET_PROC(smoke_api::steamapi_module, FUNC); \
    return _##FUNC(__VA_ARGS__)

DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(const AppId_t unOwnAppID) {
    LOG_INFO("{} -> unOwnAppID: {}", __func__, unOwnAppID);

    // Restart can be suppressed if needed

    AUTO_CALL(SteamAPI_RestartAppIfNecessary, unOwnAppID);
}

DLL_EXPORT(void) SteamAPI_Shutdown() {
    LOG_INFO("{} -> Game requested shutdown", __func__);

    AUTO_CALL(SteamAPI_Shutdown);
}
