#include <steam_impl/steam_client.hpp>
#include <steam_functions/steam_functions.hpp>

VIRTUAL(void*) ISteamClient_GetISteamApps(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* version
    )
) {
    return steam_client::GetGenericInterface(
        __func__, version, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamApps)

            return ISteamClient_GetISteamApps_o(ARGS(hSteamUser, hSteamPipe, version));
        }
    );
}

VIRTUAL(void*) ISteamClient_GetISteamUser(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* version
    )
) {
    return steam_client::GetGenericInterface(
        __func__, version, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamUser)

            return ISteamClient_GetISteamUser_o(ARGS(hSteamUser, hSteamPipe, version));
        }
    );
}

VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) {
    return steam_client::GetGenericInterface(
        __func__, pchVersion, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamGenericInterface)

            return ISteamClient_GetISteamGenericInterface_o(ARGS(hSteamUser, hSteamPipe, pchVersion));
        }
    );
}

VIRTUAL(void*) ISteamClient_GetISteamInventory(
    PARAMS(
        HSteamUser hSteamUser,
        HSteamPipe hSteamPipe,
        const char* pchVersion
    )
) {
    return steam_client::GetGenericInterface(
        __func__, pchVersion, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(ISteamClient_GetISteamInventory)

            return ISteamClient_GetISteamInventory_o(ARGS(hSteamUser, hSteamPipe, pchVersion));
        }
    );
}
