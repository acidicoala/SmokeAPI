#pragma once

#include "smoke_api/types.hpp"

namespace steam_apps {
    bool IsDlcUnlocked(
        const std::string& function_name,
        AppId_t app_id,
        AppId_t dlc_id,
        const std::function<bool()>& original_function
    );

    int GetDLCCount(
        const std::string& function_name,
        AppId_t app_id,
        const std::function<int()>& original_function
    );

    bool GetDLCDataByIndex(
        const std::string& function_name,
        AppId_t app_id,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const std::function<bool()>& original_function,
        const std::function<bool(AppId_t)>& is_originally_unlocked
        // Aux function to resolve original dlc status
    );
}
