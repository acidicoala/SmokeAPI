#include <steam_api_virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_user.hpp>

VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID steamID, AppId_t appID)) {
    return steam_user::UserHasLicenseForApp(
        __func__, appID, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamUser_UserHasLicenseForApp)

            return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, appID));
        }
    );
}
