// Not included in project due atm due to lack of usage

#include <koalabox/logger.hpp>

#include "steam_api/exports/steam_api.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/smoke_api.hpp"

#define AUTO_CALL_RETURN(FUNC, ...) \
    static const auto _##FUNC = smoke_api::hook_mode \
        ? KB_HOOK_GET_HOOKED_FN(FUNC) \
        : KB_WIN_GET_PROC(smoke_api::steamapi_module, FUNC); \
    return _##FUNC(__VA_ARGS__)

#define AUTO_CALL_RESULT(FUNC, ...) \
    static const auto _##FUNC = smoke_api::hook_mode \
        ? KB_HOOK_GET_HOOKED_FN(FUNC) \
        : KB_WIN_GET_PROC(smoke_api::steamapi_module, FUNC); \
    const auto result = _##FUNC(__VA_ARGS__)

DLL_EXPORT(bool) SteamAPI_Init() {
    LOG_INFO(__func__);

    AUTO_CALL_RESULT(SteamAPI_Init);

    LOG_INFO("{} -> result: {}", __func__, result);

    return result;
}

DLL_EXPORT(bool) SteamAPI_InitSafe() {
    LOG_INFO(__func__);

    AUTO_CALL_RESULT(SteamAPI_InitSafe);

    LOG_INFO("{} -> result: {}", __func__, result);

    return result;
}

DLL_EXPORT(ESteamAPIInitResult) SteamAPI_InitFlat(const SteamErrMsg* pOutErrMsg) {
    LOG_INFO(__func__);

    AUTO_CALL_RESULT(SteamAPI_InitFlat, pOutErrMsg);

    const auto error_message = pOutErrMsg && *pOutErrMsg
                                   ? std::string_view(*pOutErrMsg)
                                   : "";

    LOG_INFO(
        "{} -> result: {}, error_message: {}",
        __func__,
        result,
        error_message
    );

    return result;
}

DLL_EXPORT(ESteamAPIInitResult) SteamInternal_SteamAPI_Init(
    const char* pszInternalCheckInterfaceVersions,
    const SteamErrMsg* pOutErrMsg
) {
    LOG_INFO(__func__);

    AUTO_CALL_RESULT(SteamInternal_SteamAPI_Init, pszInternalCheckInterfaceVersions, pOutErrMsg);

    const auto error_message = pOutErrMsg && *pOutErrMsg
                                   ? std::string_view(*pOutErrMsg)
                                   : "";

    LOG_INFO(
        "{} -> pszInternalCheckInterfaceVersions: {}, result: {}, error_message: {}",
        __func__,
        pszInternalCheckInterfaceVersions,
        result,
        error_message
    );

    return result;
}

DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(const AppId_t unOwnAppID) {
    LOG_INFO(__func__);

    AUTO_CALL_RESULT(SteamAPI_RestartAppIfNecessary, unOwnAppID);

    LOG_INFO("{} -> unOwnAppID: {}, result: {}", __func__, unOwnAppID, result);

    // Restart can be suppressed if needed
    return result;
}

DLL_EXPORT(void) SteamAPI_Shutdown() {
    LOG_INFO("{} -> Game requested shutdown", __func__);

    AUTO_CALL_RETURN(SteamAPI_Shutdown);
}
