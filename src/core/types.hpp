#pragma once

#include <koalabox/core.hpp>
#include <nlohmann/json.hpp>

#ifdef _WIN64
#define COMPILE_KOALAGEDDON 0
#else
#define COMPILE_KOALAGEDDON 1
#endif

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
    k_EUserHasLicenseResultHasLicense = 0,         // User has a license for specified app
    k_EUserHasLicenseResultDoesNotHaveLicense = 1, // User does not have a license for the specified app
    k_EUserHasLicenseResultNoAuth = 2,             // User has not been authenticated
};

class ISteamClient;

class ISteamApps;

class ISteamUser;

class ISteamInventory;

// These aliases exist solely to increase code readability

using AppIdKey = String;
using DlcIdKey = String;
using DlcNameValue = String;
using DlcNameMap = Map<DlcIdKey, DlcNameValue>;

struct App {
    DlcNameMap dlcs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(App, dlcs) // NOLINT(misc-const-correctness)
};

using AppDlcNameMap = Map<AppIdKey, App>;

class DLC {
private:
    // These 2 names must match the property names from Steam API
    String appid;
    String name;
public:
    explicit DLC() = default;

    explicit DLC(String appid, String name) : appid{std::move(appid)}, name{std::move(name)} {}

    [[nodiscard]] String get_id_str() const {
        return appid;
    };

    [[nodiscard]] uint32_t get_id() const {
        return std::stoi(appid);
    };

    [[nodiscard]] String get_name() const {
        return name;
    };

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DLC, appid, name)

    static Vector<DLC> get_dlcs_from_apps(const AppDlcNameMap& apps, AppId_t app_id);
    static DlcNameMap get_dlc_map_from_vector(const Vector<DLC>& vector);
};
