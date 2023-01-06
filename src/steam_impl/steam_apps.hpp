#include <steam_functions/steam_functions.hpp>

namespace steam_apps {
    using namespace koalabox;

    bool IsDlcUnlocked(
        const String& function_name,
        AppId_t app_id,
        AppId_t dlc_id,
        const std::function<bool()>& original_function
    );

    int GetDLCCount(
        const String& function_name,
        AppId_t app_id,
        const std::function<int()>& original_function
    );

    bool GetDLCDataByIndex(
        const String& dlc_id,
        AppId_t dlc_ids,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const std::function<bool()>& original_function
    );

}
