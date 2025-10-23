#include <koalabox/logger.hpp>
#include <koalabox/lib.hpp>

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/interfaces/steam_user_stats.hpp"

// Flat API wrappers for ISteamUserStats
// These override the auto-generated proxy exports

extern "C" {

__declspec(dllexport) bool SteamAPI_ISteamUserStats_GetAchievement(
    void* self,
    const char* pchName,
    bool* pbAchieved
) {
    using FuncPtr = decltype(&SteamAPI_ISteamUserStats_GetAchievement);
    static FuncPtr original = nullptr;
    
    if (!original) {
        auto lib = smoke_api::get_original_library();
        if (lib) {
            original = reinterpret_cast<FuncPtr>(
                koalabox::lib::get_function_address_or_throw(lib, "SteamAPI_ISteamUserStats_GetAchievement")
            );
        }
    }
    
    if (!original) {
        return false; // Library not yet loaded
    }

    return smoke_api::steam_user_stats::GetAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        pbAchieved,
        [=]() { return original(self, pchName, pbAchieved); }
    );
}

__declspec(dllexport) bool SteamAPI_ISteamUserStats_SetAchievement(
    void* self,
    const char* pchName
) {
    using FuncPtr = decltype(&SteamAPI_ISteamUserStats_SetAchievement);
    static FuncPtr original = nullptr;
    
    if (!original) {
        auto lib = smoke_api::get_original_library();
        if (lib) {
            original = reinterpret_cast<FuncPtr>(
                koalabox::lib::get_function_address_or_throw(lib, "SteamAPI_ISteamUserStats_SetAchievement")
            );
        }
    }
    
    if (!original) {
        return false; // Library not yet loaded
    }

    return smoke_api::steam_user_stats::SetAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        [=]() { return original(self, pchName); }
    );
}

__declspec(dllexport) bool SteamAPI_ISteamUserStats_ClearAchievement(
    void* self,
    const char* pchName
) {
    using FuncPtr = decltype(&SteamAPI_ISteamUserStats_ClearAchievement);
    static FuncPtr original = nullptr;
    
    if (!original) {
        auto lib = smoke_api::get_original_library();
        if (lib) {
            original = reinterpret_cast<FuncPtr>(
                koalabox::lib::get_function_address_or_throw(lib, "SteamAPI_ISteamUserStats_ClearAchievement")
            );
        }
    }
    
    if (!original) {
        return false; // Library not yet loaded
    }

    return smoke_api::steam_user_stats::ClearAchievement(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        [=]() { return original(self, pchName); }
    );
}

__declspec(dllexport) bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(
    void* self,
    const char* pchName,
    uint32_t nCurProgress,
    uint32_t nMaxProgress
) {
    using FuncPtr = decltype(&SteamAPI_ISteamUserStats_IndicateAchievementProgress);
    static FuncPtr original = nullptr;
    
    if (!original) {
        auto lib = smoke_api::get_original_library();
        if (lib) {
            original = reinterpret_cast<FuncPtr>(
                koalabox::lib::get_function_address_or_throw(lib, "SteamAPI_ISteamUserStats_IndicateAchievementProgress")
            );
        }
    }
    
    if (!original) {
        return false; // Library not yet loaded
    }

    return smoke_api::steam_user_stats::IndicateAchievementProgress(
        __func__,
        smoke_api::get_app_id(),
        pchName,
        nCurProgress,
        nMaxProgress,
        [=]() { return original(self, pchName, nCurProgress, nMaxProgress); }
    );
}

__declspec(dllexport) bool SteamAPI_ISteamUserStats_StoreStats(void* self) {
    using FuncPtr = decltype(&SteamAPI_ISteamUserStats_StoreStats);
    static FuncPtr original = nullptr;
    
    if (!original) {
        auto lib = smoke_api::get_original_library();
        if (lib) {
            original = reinterpret_cast<FuncPtr>(
                koalabox::lib::get_function_address_or_throw(lib, "SteamAPI_ISteamUserStats_StoreStats")
            );
        }
    }
    
    if (!original) {
        return false; // Library not yet loaded
    }

    return smoke_api::steam_user_stats::StoreStats(
        __func__,
        smoke_api::get_app_id(),
        [=]() { return original(self); }
    );
}

} // extern "C"
