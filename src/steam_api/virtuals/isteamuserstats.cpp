#include <koalabox/logger.hpp>

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/interfaces/steam_user_stats.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(bool) ISteamUserStats_GetAchievement(
    PARAMS(const char* pchName, bool* pbAchieved)
) noexcept {
    return smoke_api::steam_user_stats::GetAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        pbAchieved,
        SWAPPED_CALL_CLOSURE(ISteamUserStats_GetAchievement, ARGS(pchName, pbAchieved))
    );
}

VIRTUAL(bool) ISteamUserStats_SetAchievement(
    PARAMS(const char* pchName)
) noexcept {
    return smoke_api::steam_user_stats::SetAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        SWAPPED_CALL_CLOSURE(ISteamUserStats_SetAchievement, ARGS(pchName))
    );
}

VIRTUAL(bool) ISteamUserStats_ClearAchievement(
    PARAMS(const char* pchName)
) noexcept {
    return smoke_api::steam_user_stats::ClearAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        SWAPPED_CALL_CLOSURE(ISteamUserStats_ClearAchievement, ARGS(pchName))
    );
}

VIRTUAL(bool) ISteamUserStats_IndicateAchievementProgress(
    PARAMS(const char* pchName, uint32_t nCurProgress, uint32_t nMaxProgress)
) noexcept {
    return smoke_api::steam_user_stats::IndicateAchievementProgress(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        nCurProgress,
        nMaxProgress,
        SWAPPED_CALL_CLOSURE(ISteamUserStats_IndicateAchievementProgress, ARGS(pchName, nCurProgress, nMaxProgress))
    );
}

VIRTUAL(bool) ISteamUserStats_StoreStats(PARAMS()) noexcept {
    return smoke_api::steam_user_stats::StoreStats(
        __func__,
        smoke_api::get_app_id(),
        SWAPPED_CALL_CLOSURE(ISteamUserStats_StoreStats, ARGS())
    );
}
