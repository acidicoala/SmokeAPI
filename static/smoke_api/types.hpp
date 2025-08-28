#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include <koalabox/hook.hpp>

#define VIRTUAL(TYPE) __declspec(noinline) TYPE __fastcall // NOLINT(*-macro-parentheses)
#define C_DECL(TYPE) extern "C" __declspec(noinline) TYPE __cdecl

// These macros are meant to be used for callbacks that should return original result

#define SWAPPED_CALL(CLASS, FUNC, ...) \
    const auto _##FUNC = KB_HOOK_GET_SWAPPED_FN(CLASS, FUNC); \
    return _##FUNC(__VA_ARGS__)

#define SWAPPED_CALL_CLOSURE(FUNC, ...) \
    [&] { SWAPPED_CALL(THIS, FUNC, __VA_ARGS__); }

/**
 * By default, virtual functions are declared with __thiscall
 * convention, which is normal since they are class members.
 * But it presents an issue for us, since we cannot pass *this
 * pointer as a function argument. This is because *this
 * pointer is passed via register ECX in __thiscall
 * convention. Hence, to resolve this issue we declare virtual
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
#define PARAMS(...) const void *RCX, __VA_ARGS__
#define ARGS(...) RCX, __VA_ARGS__
#define THIS RCX
#else
#define PARAMS(...) const void *ECX, const void *EDX, __VA_ARGS__
#define ARGS(...) ECX, EDX, __VA_ARGS__
#define THIS ECX
#endif

using AppId_t = uint32_t;
using HSteamPipe = uint32_t;
using HSteamUser = uint32_t;
using SteamInventoryResult_t = uint32_t;
using SteamItemInstanceID_t = uint64_t;
using SteamItemDef_t = uint32_t;
using CSteamID = uint64_t;
using HTTPRequestHandle = uint32_t;

enum class EResult {
    k_EResultNone = 0,
    k_EResultOK   = 1,
    k_EResultFail = 2,
    // See all at steamclientpublic.h
};

struct SteamItemDetails_t {
    SteamItemInstanceID_t m_itemId;
    uint32_t m_iDefinition;
    uint16_t m_unQuantity;
    uint16_t m_unFlags; // see ESteamItemFlags
};

// results from UserHasLicenseForApp
enum EUserHasLicenseForAppResult {
    // User has a license for specified app
    k_EUserHasLicenseResultHasLicense = 0,
    // User does not have a license for the specified app
    k_EUserHasLicenseResultDoesNotHaveLicense = 1,
    // User has not been authenticated
    k_EUserHasLicenseResultNoAuth = 2,
};

// These aliases exist solely to increase code readability

using AppIdKey = std::string;
using DlcIdKey = std::string;
using DlcNameValue = std::string;
using DlcNameMap = std::map<DlcIdKey, DlcNameValue>;

struct App {
    DlcNameMap dlcs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(App, dlcs)
};

using AppDlcNameMap = std::map<AppIdKey, App>;

class DLC {
    // These 2 names must match the property names from Steam API
    std::string appid;
    std::string name;

public:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DLC, appid, name)

    explicit DLC() = default;

    explicit DLC(std::string appid, std::string name) : appid{std::move(appid)},
                                                        name{std::move(name)} {}

    [[nodiscard]] std::string get_id_str() const {
        return appid;
    }

    [[nodiscard]] AppId_t get_id() const {
        return std::stoi(appid);
    }

    [[nodiscard]] std::string get_name() const {
        return name;
    }

    static std::vector<DLC> get_dlcs_from_apps(const AppDlcNameMap& apps, AppId_t app_id);

    static DlcNameMap get_dlc_map_from_vector(const std::vector<DLC>& dlcs);
};
