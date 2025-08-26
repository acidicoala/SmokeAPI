#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_user.hpp"
#include "steam_api/steam_interface.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(EUserHasLicenseForAppResult) ISteamGameServer_UserHasLicenseForApp(
    PARAMS(const CSteamID steamID, const AppId_t dlc_id)
) {
    try {
        return smoke_api::steam_user::UserHasLicenseForApp(
            __func__,
            steam_interface::get_app_id(),
            dlc_id,
            HOOKED_CALL_CLOSURE(ISteamGameServer_UserHasLicenseForApp, ARGS(steamID, dlc_id))
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return k_EUserHasLicenseResultDoesNotHaveLicense;
    }
}
