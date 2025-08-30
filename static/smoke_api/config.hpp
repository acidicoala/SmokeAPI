#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api::config {
    enum class AppStatus {
        UNDEFINED,
        ORIGINAL,
        UNLOCKED,
        LOCKED,
    };

    // @formatter:off
    NLOHMANN_JSON_SERIALIZE_ENUM(AppStatus, {
        { AppStatus::UNDEFINED,  nullptr    },
        { AppStatus::ORIGINAL,   "original" },
        { AppStatus::UNLOCKED,   "unlocked" },
        { AppStatus::LOCKED,     "locked"   },
    }) // @formatter:on

    struct Config {
        uint32_t $version = 4;
        bool logging = false;
        bool log_steam_http = false;
        AppStatus default_app_status = AppStatus::UNLOCKED;
        std::map<std::string, AppStatus> override_app_status;
        std::map<std::string, AppStatus> override_dlc_status;
        AppDlcNameMap extra_dlcs;
        bool auto_inject_inventory = true;
        std::vector<uint32_t> extra_inventory_items;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config,
            $version,
            logging,
            log_steam_http,
            default_app_status,
            override_app_status,
            override_dlc_status,
            extra_dlcs,
            auto_inject_inventory,
            extra_inventory_items
        )
    };

    extern Config instance;

    std::vector<DLC> get_extra_dlcs(AppId_t app_id);

    bool is_dlc_unlocked(
        AppId_t app_id,
        AppId_t dlc_id,
        const std::function<bool()>& original_function
    );
}
