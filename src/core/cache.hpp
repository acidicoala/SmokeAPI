#pragma once

#include <koalabox/koalabox.hpp>
#include <nlohmann/json.hpp>
#include <koalageddon/koalageddon.hpp>
#include <steam_types/steam_types.hpp>

/**
 * This namespace contains utility functions for reading from and writing to cache file on disk.
 * All functions are intended to be safe to call, i.e. they should not throw exceptions.
 */
namespace cache {
    using namespace koalabox;

    struct App {
        Vector<AppId_t> dlc_ids;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(App, dlc_ids) // NOLINT(misc-const-correctness)
    };

    struct Cache {
        // Key represents App ID
        Map<String, App> apps;
        nlohmann::json koalageddon_config;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Cache, apps, koalageddon_config) // NOLINT(misc-const-correctness)
    };

    Vector<AppId_t> get_dlc_ids(AppId_t app_id);

    std::optional<koalageddon::KoalageddonConfig> get_koalageddon_config();

    void save_dlc_ids(AppId_t app_id, const Vector<AppId_t>& dlc_ids);

    void save_koalageddon_config(const koalageddon::KoalageddonConfig& config);
}
