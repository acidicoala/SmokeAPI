#pragma once

#include <core/types.hpp>
#include <koalabox/core.hpp>

namespace smoke_api::config {

    enum class AppStatus {
        UNDEFINED,
        ORIGINAL,
        UNLOCKED,
        LOCKED,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(AppStatus, {
        { AppStatus::UNDEFINED, nullptr },
        { AppStatus::ORIGINAL, "original" },
        { AppStatus::UNLOCKED, "unlocked" },
        { AppStatus::LOCKED, "locked" },
    })

    struct Config {
        uint32_t $version = 2;
        bool logging = false;
        bool unlock_family_sharing = true;
        AppStatus default_app_status = AppStatus::UNLOCKED;
        Map<String, AppStatus> override_app_status;
        AppDlcNameMap extra_dlcs;
        bool auto_inject_inventory = true;
        Vector<uint32_t> extra_inventory_items;
        // We have to use general json type here since the library doesn't support std::optional
        Json koalageddon_config;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config, // NOLINT(misc-const-correctness)
            $version,
            logging,
            unlock_family_sharing,
            default_app_status,
            override_app_status,
            extra_dlcs,
            auto_inject_inventory,
            extra_inventory_items,
            koalageddon_config
        )
    };

    extern Config instance;

    void init();

    bool is_dlc_unlocked(uint32_t app_id, uint32_t dlc_id, const Function<bool()>& original_function);

    Vector<DLC> get_extra_dlcs(AppId_t app_id);
}
