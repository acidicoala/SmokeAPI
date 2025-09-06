#include <koalabox/logger.hpp>

#include "steam_api/steam_client.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(void*) ISteamClient_GetISteamApps(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) noexcept {
    return steam_client::GetGenericInterface(
        __func__,
        pchVersion,
        SWAPPED_CALL_CLOSURE(ISteamClient_GetISteamApps, ARGS(hSteamUser, hSteamPipe, pchVersion))
    );
}

VIRTUAL(void*) ISteamClient_GetISteamUser(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) noexcept {
    return steam_client::GetGenericInterface(
        __func__,
        pchVersion,
        SWAPPED_CALL_CLOSURE(ISteamClient_GetISteamUser, ARGS(hSteamUser, hSteamPipe, pchVersion))
    );
}

VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) noexcept {
    return steam_client::GetGenericInterface(
        __func__,
        pchVersion,
        SWAPPED_CALL_CLOSURE(
            ISteamClient_GetISteamGenericInterface,
            ARGS(hSteamUser, hSteamPipe, pchVersion)
        )
    );
}

VIRTUAL(void*) ISteamClient_GetISteamInventory(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) noexcept {
    return steam_client::GetGenericInterface(
        __func__,
        pchVersion,
        SWAPPED_CALL_CLOSURE(
            ISteamClient_GetISteamInventory,
            ARGS(hSteamUser, hSteamPipe, pchVersion)
        )
    );
}
