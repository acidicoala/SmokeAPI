#pragma once

#include "smoke_api/types.hpp"

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
#define PARAMS(...) void *RCX, __VA_ARGS__
#define ARGS(...) RCX, __VA_ARGS__
#define THIS RCX
#else
#define PARAMS(...) const void *ECX, const void *EDX, __VA_ARGS__
#define ARGS(...) ECX, EDX, __VA_ARGS__
#define THIS ECX
#endif

#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall // NOLINT(*-macro-parentheses)

// ISteamApps
VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t));
VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t));
VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS());
VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(PARAMS(int, AppId_t*, bool*, char*, int));

// ISteamClient
VIRTUAL(void*) ISteamClient_GetISteamApps(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamUser(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamInventory(PARAMS(HSteamUser, HSteamPipe, const char*));

// ISteamInventory
VIRTUAL(EResult) ISteamInventory_GetResultStatus(PARAMS(SteamInventoryResult_t));
VIRTUAL(bool) ISteamInventory_GetResultItems(
    PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*) // @formatter:off
); // @formatter:on

VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*) // @formatter:off
); // @formatter:on
VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t*));
VIRTUAL(bool) ISteamInventory_GetItemsByID(
    PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t)
);
VIRTUAL(bool) ISteamInventory_SerializeResult(PARAMS(SteamInventoryResult_t, void*, uint32_t*));
VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t*));
VIRTUAL(bool) ISteamInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID));

// ISteamUser
VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID, AppId_t));
