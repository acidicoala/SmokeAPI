#pragma once

#include <koalabox/core.hpp>
#include <koalabox/json.hpp>

namespace config {
    enum class AppStatus {
        LOCKED,
        UNLOCKED,
        UNDEFINED
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(AppStatus, {
        { AppStatus::UNDEFINED, nullptr },
        { AppStatus::LOCKED, "locked" },
        { AppStatus::UNLOCKED, "unlocked" },
    })

    enum class DlcStatus {
        LOCKED,
        UNLOCKED,
        ORIGINAL,
        UNDEFINED
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(DlcStatus, {
        { DlcStatus::UNDEFINED, nullptr },
        { DlcStatus::LOCKED, "locked" },
        { DlcStatus::UNLOCKED, "unlocked" },
        { DlcStatus::ORIGINAL, "original" },
    })

    struct Config {
        uint32_t $version = 2;
        bool logging = false;
        bool unlock_family_sharing = true;
        AppStatus default_app_status = AppStatus::UNLOCKED;
        DlcStatus default_dlc_status = DlcStatus::UNLOCKED;
        Map<String, AppStatus> override_app_status;
        Map<String, DlcStatus> override_dlc_status;
        Vector<uint32_t> extra_dlc_ids;
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
            default_dlc_status,
            override_app_status,
            override_dlc_status,
            extra_dlc_ids,
            auto_inject_inventory,
            extra_inventory_items,
            koalageddon_config
        )
    };

    extern Config instance;

    void init();

    AppStatus get_app_status(uint32_t app_id);

    DlcStatus get_dlc_status(uint32_t dlc_id);

    bool is_dlc_unlocked(uint32_t app_id, uint32_t dlc_id, const Function<bool()>& original_function);
}
