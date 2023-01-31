#pragma once

#include <core/types.hpp>

namespace steam_apps {

    bool IsDlcUnlocked(
        const String& function_name,
        AppId_t app_id,
        AppId_t dlc_id,
        const Function<bool()>& original_function
    );

    int GetDLCCount(
        const String& function_name,
        AppId_t app_id,
        const Function<int()>& original_function
    );

    bool GetDLCDataByIndex(
        const String& function_name,
        AppId_t app_id,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const Function<bool()>& original_function,
        const Function<bool(AppId_t)>& is_originally_unlocked // Aux function to resolve original dlc status
    );

}
