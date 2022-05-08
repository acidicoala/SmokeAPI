#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

using namespace smoke_api;

VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID steamID, AppId_t appID)) {
    return steam_user::UserHasLicenseForApp(__func__, appID, [&]() {
        GET_ORIGINAL_VIRTUAL_FUNCTION(ISteamUser_UserHasLicenseForApp)

        return ISteamUser_UserHasLicenseForApp_o(ARGS(steamID, appID));
    });
}
