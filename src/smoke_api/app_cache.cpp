#include <smoke_api/app_cache.hpp>
#include <core/paths.hpp>
#include <koalabox/cache.hpp>
#include <koalabox/logger.hpp>

struct App {
    Vector<AppId_t> dlc_ids;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(App, dlc_ids) // NOLINT(misc-const-correctness)
};

using Apps = Map<String, App>;

constexpr auto KEY_APPS = "apps";

Apps get_cached_apps() {
    try {
        const auto cache = koalabox::cache::read_from_cache(KEY_APPS);

        return cache.get<Apps>();
    } catch (const Exception& e) {
        LOG_WARN("Failed to get cached apps: {}", e.what())

        return {};
    }
}

namespace smoke_api::app_cache {

    Vector<AppId_t> get_dlc_ids(AppId_t app_id) {
        try {
            LOG_DEBUG("Reading cached DLC IDs for the app: {}", app_id)

            const auto app = get_cached_apps().at(std::to_string(app_id));

            return app.dlc_ids;
        } catch (const Exception& e) {
            return {};
        }
    }

    bool save_dlc_ids(AppId_t app_id, const Vector<AppId_t>& dlc_ids) {
        try {
            LOG_DEBUG("Caching DLC IDs for the app: {}", app_id)

            auto apps = get_cached_apps();

            apps[std::to_string(app_id)] = {
                .dlc_ids = dlc_ids
            };

            return koalabox::cache::save_to_cache(KEY_APPS, Json(apps));
        } catch (const Exception& e) {
            LOG_ERROR("Failed to cache DLC IDs fro the app: {}", app_id)

            return false;
        }
    }

}
