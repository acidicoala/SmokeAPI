#include <koalabox/logger.hpp>

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/interfaces/steam_user.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(EUserHasLicenseForAppResult) ISteamGameServer_UserHasLicenseForApp(
    PARAMS(const CSteamID steamID, const AppId_t dlc_id)
) noexcept {
    return smoke_api::steam_user::UserHasLicenseForApp(
        __func__,
        smoke_api::get_app_id(),
        dlc_id,
        SWAPPED_CALL_CLOSURE(ISteamGameServer_UserHasLicenseForApp, ARGS(steamID, dlc_id))
    );
}
