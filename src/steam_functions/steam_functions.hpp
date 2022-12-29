#pragma once

#include <koalabox/koalabox.hpp>

#include <steam_types/steam_types.hpp>

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
#define PARAMS(...) void* RCX, ##__VA_ARGS__
#define ARGS(...) RCX, ##__VA_ARGS__
#define THIS RCX
#else
#define PARAMS(...) void* ECX, void* EDX, ##__VA_ARGS__
#define ARGS(...) ECX, EDX, ##__VA_ARGS__
#define THIS ECX
#endif

#define DLL_EXPORT(TYPE) extern "C" __declspec( dllexport ) TYPE __cdecl
#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall

class ISteamClient;

class ISteamApps;

class ISteamUser;

class ISteamInventory;

typedef __int32 HSteamPipe;
typedef __int32 HSteamUser;
typedef uint32_t AppId_t;
typedef uint64_t CSteamID;

typedef uint32_t HCoroutine;

// ISteamClient
VIRTUAL(void*) ISteamClient_GetISteamApps(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamUser(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamGenericInterface(PARAMS(HSteamUser, HSteamPipe, const char*));
VIRTUAL(void*) ISteamClient_GetISteamInventory(PARAMS(HSteamUser, HSteamPipe, const char*));

// ISteamApps
VIRTUAL(bool) ISteamApps_BIsSubscribedApp(PARAMS(AppId_t));
VIRTUAL(bool) ISteamApps_BIsDlcInstalled(PARAMS(AppId_t));
VIRTUAL(int) ISteamApps_GetDLCCount(PARAMS());
VIRTUAL(bool) ISteamApps_BGetDLCDataByIndex(PARAMS(int, AppId_t*, bool*, char*, int));

// ISteamUser
VIRTUAL(EUserHasLicenseForAppResult) ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID, AppId_t));

// ISteamInventory
VIRTUAL(EResult) ISteamInventory_GetResultStatus(PARAMS(uint32_t));
VIRTUAL(bool) ISteamInventory_GetResultItems(PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*));
VIRTUAL(bool) ISteamInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*)
);
VIRTUAL(bool) ISteamInventory_GetAllItems(PARAMS(SteamInventoryResult_t*));
VIRTUAL(bool) ISteamInventory_GetItemsByID(PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t));
VIRTUAL(bool) ISteamInventory_SerializeResult(PARAMS(SteamInventoryResult_t, void*, uint32_t*));
VIRTUAL(bool) ISteamInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t*));
VIRTUAL(bool) ISteamInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID));

// API

DLL_EXPORT(void*) CreateInterface(const char*, int*);
DLL_EXPORT(void*) SteamInternal_FindOrCreateUserInterface(HSteamUser, const char*);
DLL_EXPORT(void*) SteamInternal_CreateInterface(const char*);
DLL_EXPORT(void*) SteamApps();
DLL_EXPORT(void*) SteamClient();
DLL_EXPORT(void*) SteamUser();

// Flat interfaces

DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps*, AppId_t);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps*, AppId_t);
DLL_EXPORT(int) SteamAPI_ISteamApps_GetDLCCount(ISteamApps*);
DLL_EXPORT(bool) SteamAPI_ISteamApps_BGetDLCDataByIndex(ISteamApps*, int, AppId_t*, bool*, char*, int);
DLL_EXPORT(EUserHasLicenseForAppResult) SteamAPI_ISteamUser_UserHasLicenseForApp(ISteamUser*, CSteamID, AppId_t);
DLL_EXPORT(void*) SteamAPI_ISteamClient_GetISteamGenericInterface(ISteamClient*, HSteamUser, HSteamPipe, const char*);
DLL_EXPORT(EResult) SteamAPI_ISteamInventory_GetResultStatus(ISteamInventory*, SteamInventoryResult_t);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItems(
    ISteamInventory*, SteamInventoryResult_t, SteamItemDetails_t*, uint32_t*
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetResultItemProperty(
    ISteamInventory*, SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t*
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_CheckResultSteamID(ISteamInventory*, SteamInventoryResult_t, CSteamID);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetAllItems(ISteamInventory*, SteamInventoryResult_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemsByID(
    ISteamInventory*, SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t
);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_SerializeResult(ISteamInventory*, SteamInventoryResult_t, void*, uint32_t*);
DLL_EXPORT(bool) SteamAPI_ISteamInventory_GetItemDefinitionIDs(ISteamInventory*, SteamItemDef_t*, uint32_t*);

// Koalageddon mode

DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, struct CoroutineData* data);
DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name);

// IClientApps
VIRTUAL(int) IClientApps_GetDLCCount(PARAMS(AppId_t));
VIRTUAL(bool) IClientApps_BGetDLCDataByIndex(PARAMS(AppId_t, int, AppId_t*, bool*, char*, int));

// IClientAppManager
VIRTUAL(bool) IClientAppManager_IsAppDlcInstalled(PARAMS(AppId_t, AppId_t));

// IClientUser
VIRTUAL(bool) IClientUser_IsSubscribedApp(PARAMS(AppId_t));

// IClientInventory
VIRTUAL(EResult) IClientInventory_GetResultStatus(PARAMS(SteamInventoryResult_t));
VIRTUAL(bool) IClientInventory_GetResultItems(
    PARAMS(SteamInventoryResult_t, SteamItemDetails_t*, uint32_t, uint32_t *)
);
//////
VIRTUAL(bool) IClientInventory_GetResultItemProperty(
    PARAMS(SteamInventoryResult_t, uint32_t, const char*, char*, uint32_t, uint32_t*)
);
VIRTUAL(bool) IClientInventory_CheckResultSteamID(PARAMS(SteamInventoryResult_t, CSteamID));
VIRTUAL(bool) IClientInventory_GetAllItems(PARAMS(SteamInventoryResult_t*));
VIRTUAL(bool) IClientInventory_GetItemsByID(PARAMS(SteamInventoryResult_t*, const SteamItemInstanceID_t*, uint32_t));
VIRTUAL(bool) IClientInventory_SerializeResult(PARAMS(SteamInventoryResult_t, void*, uint32_t, uint32_t *));
VIRTUAL(bool) IClientInventory_GetItemDefinitionIDs(PARAMS(SteamItemDef_t*, uint32_t, uint32_t *));

namespace steam_functions {
    using namespace koalabox;

    const String STEAM_APPS = "STEAMAPPS_INTERFACE_VERSION"; // NOLINT(cert-err58-cpp)
    const String STEAM_CLIENT = "SteamClient"; // NOLINT(cert-err58-cpp)
    const String STEAM_USER = "SteamUser"; // NOLINT(cert-err58-cpp)
    const String STEAM_INVENTORY = "STEAMINVENTORY_INTERFACE_V"; // NOLINT(cert-err58-cpp)

    void hook_virtuals(void* interface, const String& version_string);
    uint32_t get_app_id_or_throw();
}
