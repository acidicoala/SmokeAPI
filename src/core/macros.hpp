#pragma once

#include <core/globals.hpp>
#include <koalabox/hook.hpp>

/**
 * By default, virtual functions are declared with __thiscall
 * convention, which is normal since they are class members.
 * But it presents an issue for us, since we cannot pass *this
 * pointer as a function argument. This is because *this
 * pointer is passed via register ECX in __thiscall
 * convention. Hence, to resolve this issue we declare our
 * hooked functions with __fastcall convention, to trick
 * the compiler into reading ECX & EDX registers as 1st
 * and 2nd function arguments respectively. Similarly, __fastcall
 * makes the compiler push the first argument into the ECX register,
 * which mimics the __thiscall calling convention. Register EDX
 * is not used anywhere in this case, but we still pass it along
 * to conform to the __fastcall convention. This all applies
 * to the x86 architecture.
 *
 * In x86-64 however, there is only one calling convention,
 * so __fastcall is simply ignored. However, RDX in this case
 * will store the 1st actual argument to the function, so we
 * have to omit it from the function signature.
 *
 * The macros below implement the above-mentioned considerations.
 */
#ifdef _WIN64
#define PARAMS(...) void* RCX, __VA_ARGS__
#define ARGS(...) RCX, __VA_ARGS__
#define THIS RCX
#else
#define PARAMS(...) const void* ECX, const void* EDX, __VA_ARGS__
#define ARGS(...) ECX, EDX, __VA_ARGS__
#define THIS ECX
#endif

// Names beginning with $ designate macros that are not meant to be used directly by the sources consuming this file

#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec( dllexport ) TYPE __cdecl
#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall

#define GET_ORIGINAL_HOOKED_FUNCTION(FUNC) \
    static const auto FUNC##_o = koalabox::hook::get_original_hooked_function(globals::address_map, #FUNC, FUNC);

#define GET_ORIGINAL_FUNCTION_STEAMAPI(FUNC) \
    static const auto FUNC##_o = koalabox::hook::get_original_function(globals::steamapi_module, #FUNC, FUNC);


#define DETOUR_ADDRESS(FUNC, ADDRESS) \
    koalabox::hook::detour_or_warn(globals::address_map, ADDRESS, #FUNC, reinterpret_cast<uintptr_t>(FUNC));

#define $DETOUR(FUNC, NAME, MODULE_HANDLE) \
    koalabox::hook::detour_or_warn(globals::address_map, MODULE_HANDLE, NAME, reinterpret_cast<uintptr_t>(FUNC));

#define DETOUR_STEAMCLIENT(FUNC) $DETOUR(FUNC, #FUNC, globals::steamclient_module)
#define DETOUR_VSTDLIB(FUNC) $DETOUR(vstdlib::FUNC, #FUNC, globals::vstdlib_module)
