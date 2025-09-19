#include <koalabox/logger.hpp>

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/interfaces/steam_user.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(AppId_t) ISteamUtils_GetAppID(PARAMS()) noexcept {
    SWAPPED_CALL(THIS, ISteamUtils_GetAppID, ARGS());
}
