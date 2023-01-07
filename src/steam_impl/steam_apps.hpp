#include <core/steam_types.hpp>
#include <koalabox/types.hpp>

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
        const String& dlc_id,
        AppId_t dlc_ids,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const Function<bool()>& original_function
    );

}
