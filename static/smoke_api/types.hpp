#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

// TODO: Replace with direct call
#define GET_ORIGINAL_HOOKED_FUNCTION(FUNC)                                                         \
    static const auto FUNC##_o = koalabox::hook::get_original_hooked_function(#FUNC, FUNC);

#define DETOUR_ADDRESS(FUNC, ADDRESS)                                                              \
    koalabox::hook::detour_or_warn(ADDRESS, #FUNC, reinterpret_cast<uintptr_t>(FUNC));

using AppId_t = uint32_t;
using HSteamPipe = uint32_t;
using EResult = uint32_t;
using HSteamUser = uint32_t;
using SteamInventoryResult_t = uint32_t;
using SteamItemInstanceID_t = uint64_t;
using SteamItemDef_t = uint32_t;
using CSteamID = uint64_t;

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

    [[nodiscard]] AppId_t get_id() const {
        return std::stoi(appid);
    }

    [[nodiscard]] std::string get_name() const {
        return name;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DLC, appid, name)

    static std::vector<DLC> get_dlcs_from_apps(const AppDlcNameMap& apps, AppId_t app_id);

    static DlcNameMap get_dlc_map_from_vector(const std::vector<DLC>& dlcs);
};