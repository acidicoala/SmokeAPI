#include <steam_functions/steam_functions.hpp>

using namespace smoke_api;

namespace steam_apps {

    bool IsDlcUnlocked(const String& function_name, AppId_t app_id, AppId_t dlc_id);

    int GetDLCCount(const String& function_name, AppId_t app_id, const std::function<int()>& original_function);

    bool GetDLCDataByIndex(
        const String& function_name,
        AppId_t app_id,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const std::function<bool()>& original_function
    );

}
