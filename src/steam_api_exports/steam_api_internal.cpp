#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_client.hpp>

using namespace smoke_api;

DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(HSteamUser hSteamUser, const char* version) {
    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamInternal_FindOrCreateUserInterface)

        return SteamInternal_FindOrCreateUserInterface_o(hSteamUser, version);
    });
}

DLL_EXPORT(void*) SteamInternal_CreateInterface(const char* version) {
    return steam_client::GetGenericInterface(__func__, version, [&]() {
        GET_ORIGINAL_FUNCTION(SteamInternal_CreateInterface)

        return SteamInternal_CreateInterface_o(version);
    });
}
