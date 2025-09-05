#pragma once

#include "smoke_api/types.hpp"

constexpr auto STEAM_APPS = "STEAMAPPS_INTERFACE_VERSION";
constexpr auto STEAM_CLIENT = "SteamClient";
constexpr auto STEAM_HTTP = "STEAMHTTP_INTERFACE_VERSION";
constexpr auto STEAM_USER = "SteamUser";
constexpr auto STEAM_INVENTORY = "STEAMINVENTORY_INTERFACE_V";
constexpr auto STEAM_GAME_SERVER = "SteamGameServer";

// IMPORTANT: DLL_EXPORT is hardcoded in exports_generator.cpp,
// so any name changes here must be reflected there as well.
#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec(dllexport) TYPE __cdecl

namespace smoke_api {

    void init(HMODULE module_handle);
    void shutdown();

    AppId_t get_app_id();
}
