#pragma once

#include <koalabox/hook.hpp>

constexpr auto STEAM_APPS = "STEAMAPPS_INTERFACE_VERSION";
constexpr auto STEAM_CLIENT = "SteamClient";
constexpr auto STEAM_USER = "SteamUser";
constexpr auto STEAM_INVENTORY = "STEAMINVENTORY_INTERFACE_V";

// IMPORTANT: DLL_EXPORT is hardcoded in exports_generator.cpp,
// so any name changes here must be reflected there as well.
#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec(dllexport) TYPE __cdecl

// These macros are meant to be used for callbacks that should return original result

#define MODULE_CALL(FUNC, ...) \
    static const auto _##FUNC = KB_HOOK_GET_MODULE_FN(smoke_api::steamapi_module, FUNC); \
    return _##FUNC(__VA_ARGS__)

#define MODULE_CALL_CLOSURE(FUNC, ...) \
    [&] { MODULE_CALL(FUNC, __VA_ARGS__); }

#define AUTO_CALL(FUNC, ...) \
   static const auto _##FUNC = smoke_api::hook_mode \
    ? KB_HOOK_GET_HOOKED_FN(FUNC) \
    : KB_HOOK_GET_MODULE_FN(smoke_api::steamapi_module, FUNC); \
    return _##FUNC(__VA_ARGS__)

namespace smoke_api {
    extern HMODULE steamapi_module;

    extern bool hook_mode;

    void init(HMODULE module_handle);

    void shutdown();
}
