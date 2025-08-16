#include <game_mode/virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_user.hpp>
#include <steam_impl/steam_impl.hpp>
#include <koalabox/logger.hpp>

VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID steamID, AppId_t dlc_id)) {
    try {
        static const auto app_id = steam_impl::get_app_id_or_throw();
        return steam_user::UserHasLicenseForApp(
            __func__, app_id, dlc_id, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamUser_UserHasLicenseForApp)

                return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, dlc_id));
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
}
