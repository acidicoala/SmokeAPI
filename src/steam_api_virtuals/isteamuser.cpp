#include <steam_api_virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_user.hpp>
#include <steam_impl/steam_impl.hpp>

VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID steamID, AppId_t dlcID)) {
    return steam_user::UserHasLicenseForApp(
        __func__, steam_impl::get_app_id_or_throw(), dlcID, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamUser_UserHasLicenseForApp)

            return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, dlcID));
        }
    );
}
