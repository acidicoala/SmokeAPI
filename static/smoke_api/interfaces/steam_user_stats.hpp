#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api::steam_user_stats {
    bool GetAchievement(
        const char* function_name,
        AppId_t app_id,
        const char* achievement_name,
        bool* achieved,
        const std::function<bool()>& original_function
    ) noexcept;

    bool SetAchievement(
        const char* function_name,
        AppId_t app_id,
        const char* achievement_name,
        const std::function<bool()>& original_function
    ) noexcept;

    bool ClearAchievement(
        const char* function_name,
        AppId_t app_id,
        const char* achievement_name,
        const std::function<bool()>& original_function
    ) noexcept;

    bool IndicateAchievementProgress(
        const char* function_name,
        AppId_t app_id,
        const char* achievement_name,
        uint32_t current_progress,
        uint32_t max_progress,
        const std::function<bool()>& original_function
    ) noexcept;

    bool StoreStats(
        const char* function_name,
        AppId_t app_id,
        const std::function<bool()>& original_function
    ) noexcept;
}
