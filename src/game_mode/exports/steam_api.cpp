#include <smoke_api/config.hpp>

// TODO: Detour in hook mode
DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(
    [[maybe_unused]] const uint32_t unOwnAppID
) {
    if (smoke_api::config::instance.override_app_id != 0) {
        LOG_DEBUG("{} -> Preventing app restart", __func__)
        return false;
    }

    return ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_RestartAppIfNecessary)(unOwnAppID);
}

DLL_EXPORT(void) SteamAPI_Shutdown() {
    LOG_INFO("{} -> Game requested shutdown", __func__)

    ORIGINAL_FUNCTION_STEAMAPI(SteamAPI_Shutdown)();
}