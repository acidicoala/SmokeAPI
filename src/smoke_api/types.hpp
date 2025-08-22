#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

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

// Names beginning with $ designate macros that are not meant to be used directly by the sources
// consuming this file

// IMPORTANT: DLL_EXPORT is hardcoded in exports_generator.cpp,
// so any name changes here must be reflected there as well.
#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec(dllexport) TYPE __cdecl

#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall
#define C_DECL(TYPE) extern "C" __declspec(noinline) TYPE __cdecl

// TODO: Replace with direct call
#define GET_ORIGINAL_HOOKED_FUNCTION(FUNC)                                                         \
    static const auto FUNC##_o = koalabox::hook::get_original_hooked_function(#FUNC, FUNC);

#define ORIGINAL_FUNCTION_STEAMAPI(FUNC)                                                           \
    koalabox::hook::get_original_function(globals::steamapi_module, #FUNC, FUNC)

// TODO: Rename to DEFINE_ORIGINAL_FUNCTION_STEAMAPI
#define GET_ORIGINAL_FUNCTION_STEAMAPI(FUNC)                                                       \
    static const auto FUNC##_o = ORIGINAL_FUNCTION_STEAMAPI(FUNC);

#define DETOUR_ADDRESS(FUNC, ADDRESS)                                                              \
    koalabox::hook::detour_or_warn(ADDRESS, #FUNC, reinterpret_cast<uintptr_t>(FUNC));

#define _DETOUR(FUNC, NAME, MODULE_HANDLE)                                                         \
    koalabox::hook::detour_or_warn(MODULE_HANDLE, NAME, reinterpret_cast<uintptr_t>(FUNC));

constexpr auto STEAM_APPS = "STEAMAPPS_INTERFACE_VERSION";
constexpr auto STEAM_CLIENT = "SteamClient";
constexpr auto STEAM_USER = "SteamUser";
constexpr auto STEAM_INVENTORY = "STEAMINVENTORY_INTERFACE_V";

using AppId_t = uint32_t;
using SteamInventoryResult_t = uint32_t;
using SteamItemInstanceID_t = uint64_t;
using SteamItemDef_t = uint32_t;
using HSteamPipe = uint32_t;
using HSteamUser = uint32_t;
using CSteamID = uint64_t;
using EResult = uint32_t;

struct SteamItemDetails_t {
    SteamItemInstanceID_t m_itemId;
    uint32_t m_iDefinition;
    uint16_t m_unQuantity;
    uint16_t m_unFlags; // see ESteamItemFlags
};

// results from UserHasLicenseForApp
enum EUserHasLicenseForAppResult {
    k_EUserHasLicenseResultHasLicense         = 0, // User has a license for specified app
    k_EUserHasLicenseResultDoesNotHaveLicense = 1,
    // User does not have a license for the specified app
    k_EUserHasLicenseResultNoAuth = 2, // User has not been authenticated
};

// These aliases exist solely to increase code readability

using AppIdKey = std::string;
using DlcIdKey = std::string;
using DlcNameValue = std::string;
using DlcNameMap = std::map<DlcIdKey, DlcNameValue>;

struct App {
    DlcNameMap dlcs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(App, dlcs) // NOLINT(misc-const-correctness)
};

using AppDlcNameMap = std::map<AppIdKey, App>;

class DLC {
    // These 2 names must match the property names from Steam API
    std::string appid;
    std::string name;

public:
    explicit DLC() = default;

    explicit DLC(std::string appid, std::string name) : appid{std::move(appid)},
                                                        name{std::move(name)} {}

    [[nodiscard]] std::string get_id_str() const {
        return appid;
    }

    [[nodiscard]] uint32_t get_id() const {
        return std::stoi(appid);
    }

    [[nodiscard]] std::string get_name() const {
        return name;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DLC, appid, name)

    static std::vector<DLC> get_dlcs_from_apps(const AppDlcNameMap& apps, AppId_t app_id);

    static DlcNameMap get_dlc_map_from_vector(const std::vector<DLC>& vector);
};