#include <koalabox/logger.hpp>

#include "smoke_api.hpp"
#include "steam_api/steam_client.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(void*) ISteamClient_GetISteamApps(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* version
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__,
            version,
            HOOKED_CALL_CLOSURE(ISteamClient_GetISteamApps, ARGS(hSteamUser, hSteamPipe, version))
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

VIRTUAL(void*) ISteamClient_GetISteamUser(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* version
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__,
            version,
            HOOKED_CALL_CLOSURE(ISteamClient_GetISteamUser, ARGS(hSteamUser, hSteamPipe, version))
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__,
            pchVersion,
            HOOKED_CALL_CLOSURE(
                ISteamClient_GetISteamGenericInterface,
                ARGS(hSteamUser, hSteamPipe, pchVersion)
            )
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

VIRTUAL(void*) ISteamClient_GetISteamInventory(
    PARAMS(
        const HSteamUser hSteamUser,
        const HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__,
            pchVersion,
            HOOKED_CALL_CLOSURE(
                ISteamClient_GetISteamInventory,
                ARGS(hSteamUser, hSteamPipe, pchVersion)
            )
        );
    } catch(const std::exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}
