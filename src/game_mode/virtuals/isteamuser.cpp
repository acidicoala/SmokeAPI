#include <game_mode/virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_user.hpp>
#include <steam_impl/steam_impl.hpp>
#include <koalabox/logger.hpp>

VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID steamID, AppId_t dlcID)) {
    AppId_t app_id = 0;
    try {
        app_id = steam_impl::get_app_id_or_throw();
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error getting app id: {}", __func__, e.what())
    }

    return steam_user::UserHasLicenseForApp(
        __func__, app_id, dlcID, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamUser_UserHasLicenseForApp)

            return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, dlcID));
        }
    );
}
