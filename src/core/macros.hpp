#pragma once

#include <core/globals.hpp>
#include <koalabox/hook.hpp>

// Names beginning with $ designate macros that are not meant to be used directly by the sources consuming this file

#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec( dllexport ) TYPE __cdecl
#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall

#define GET_ORIGINAL_HOOKED_FUNCTION(FUNC) \
    static const auto FUNC##_o = koalabox::hook::get_original_hooked_function(globals::address_map, #FUNC, FUNC);

#define GET_ORIGINAL_FUNCTION_STEAMAPI(FUNC) \
    static const auto FUNC##_o = koalabox::hook::get_original_function(globals::steamapi_module, #FUNC, FUNC);

#define $DETOUR(FUNC, MODULE_HANDLE) \
    koalabox::hook::detour_or_warn(globals::address_map, MODULE_HANDLE, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));

#define DETOUR_ADDRESS(FUNC, ADDRESS) \
    koalabox::hook::detour_or_warn(globals::address_map, ADDRESS, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));

#define DETOUR_STEAMCLIENT(FUNC) $DETOUR(FUNC, globals::steamclient_module)
#define DETOUR_VSTDLIB(FUNC) $DETOUR(FUNC, globals::vstdlib_module)
