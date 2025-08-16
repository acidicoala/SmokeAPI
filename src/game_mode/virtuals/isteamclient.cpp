#include <game_mode/virtuals/steam_api_virtuals.hpp>
#include <steam_impl/steam_client.hpp>

VIRTUAL(void*) ISteamClient_GetISteamApps(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* version
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__, version, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamApps)
                return ISteamClient_GetISteamApps_o(ARGS(hSteamUser, hSteamPipe, version));
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

VIRTUAL(void*) ISteamClient_GetISteamUser(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* version
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__, version, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamUser)

                return ISteamClient_GetISteamUser_o(ARGS(hSteamUser, hSteamPipe, version));
            }
        );
    } catch (const Exception& e) {
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
            __func__, pchVersion, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamGenericInterface)

                return ISteamClient_GetISteamGenericInterface_o(ARGS(hSteamUser, hSteamPipe, pchVersion));
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}

VIRTUAL(void*) ISteamClient_GetISteamInventory(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) {
    try {
        return steam_client::GetGenericInterface(
            __func__, pchVersion, [&]() {
                GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamInventory)

                return ISteamClient_GetISteamInventory_o(ARGS(hSteamUser, hSteamPipe, pchVersion));
            }
        );
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what());
        return nullptr;
    }
}
