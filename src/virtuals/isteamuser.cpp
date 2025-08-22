#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>

#include "steam_interface/steam_interface.hpp"
#include "steam_interface/steam_user.hpp"
#include "virtuals/steam_api_virtuals.hpp"

VIRTUAL (EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(
            PARAMS(CSteamID steamID, AppId_t dlc_id)
        ) {
    try {
        static const auto app_id = steam_interface::get_app_id();
        return steam_user::UserHasLicenseForApp(
            __func__,
            app_id,
            dlc_id,
            [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamUser_UserHasLicenseForApp)

                return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, dlc_id));
            }
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
}