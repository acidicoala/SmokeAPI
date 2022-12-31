#pragma once

#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec( dllexport ) TYPE __cdecl
#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall

#define DETOUR(FUNC, ADDRESS) hook::detour_or_warn(ADDRESS, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));
#define DETOUR_ORIGINAL(FUNC) DETOUR(FUNC, smoke_api::original_library)
