#include <koalabox/cache.hpp>
#include <koalabox/logger.hpp>

#include "smoke_api/cache.hpp"

constexpr auto KEY_APPS = "apps";

namespace {
    AppDlcNameMap get_cached_apps() {
        try {
            return koalabox::cache::get(KEY_APPS).get<AppDlcNameMap>();
        } catch(const std::exception& e) {
            LOG_WARN("Failed to get cached apps: {}", e.what());
            return {};
        }
    }
}

namespace smoke_api::cache {
    std::vector<DLC> get_dlcs(AppId_t app_id) noexcept {
        try {
            LOG_DEBUG("Reading cached DLC IDs for the app: {}", app_id);

            const auto apps = get_cached_apps();

            return DLC::get_dlcs_from_apps(apps, app_id);
        } catch(const std::exception& e) {
            LOG_ERROR("Error reading DLCs from disk cache: ", e.what());

            return {};
        }
    }

    bool save_dlcs(AppId_t app_id, const std::vector<DLC>& dlcs) noexcept {
        static std::mutex section;
        const std::lock_guard lock(section);

        try {
            LOG_DEBUG("Caching DLC IDs for the app: {}", app_id);

            auto apps = get_cached_apps();

            apps[std::to_string(app_id)] = App{.dlcs = DLC::get_dlc_map_from_vector(dlcs)};

            return koalabox::cache::put(KEY_APPS, nlohmann::json(apps));
        } catch(const std::exception& e) {
            LOG_ERROR("Error saving DLCs to disk cache: {}", e.what());

            return false;
        }
    }
}
