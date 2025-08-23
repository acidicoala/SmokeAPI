#include "smoke_api.hpp"
#include "smoke_api/types.hpp"
#include "steam_api/steam_client.hpp"

DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(
    const HSteamUser hSteamUser,
    const char* version
) {
    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamInternal_FindOrCreateUserInterface, hSteamUser, version)
    );
}

DLL_EXPORT(void*) SteamInternal_CreateInterface(const char* version) {
    return steam_client::GetGenericInterface(
        __func__,
        version,
        MODULE_CALL_CLOSURE(SteamInternal_CreateInterface, version)
    );
}
