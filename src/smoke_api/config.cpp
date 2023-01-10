#include <smoke_api/config.hpp>
#include <core/paths.hpp>
#include <koalabox/util.hpp>
#include <koalabox/io.hpp>
#include <koalabox/logger.hpp>

namespace smoke_api::config {
    Config instance; // NOLINT(cert-err58-cpp)

    // TODO: Reloading via export
    void init() {
        const auto path = paths::get_config_path();

        if (exists(path)) {
            try {
                instance = Json::parse(koalabox::io::read_file(path)).get<Config>();
            } catch (const Exception& e) {
                koalabox::util::panic("Error parsing config: {}", e.what());
            }
        }
    }

    bool is_dlc_unlocked(AppId_t app_id, AppId_t dlc_id, const Function<bool()>& original_function) {
        const auto app_id_str = std::to_string(app_id);
        const auto dlc_id_str = std::to_string(dlc_id);

        auto status = instance.default_app_status;

        if (instance.override_app_status.contains(app_id_str)) {
            status = instance.override_app_status[app_id_str];
        }

        if (instance.override_app_status.contains(dlc_id_str)) {
            status = instance.override_app_status[dlc_id_str];
        }

        bool is_unlocked;
        switch (status) {
            case AppStatus::UNLOCKED:
                is_unlocked = true;
                break;
            case AppStatus::LOCKED:
                is_unlocked = false;
                break;
            case AppStatus::ORIGINAL:
            case AppStatus::UNDEFINED:
                is_unlocked = original_function();
                break;
        }
        LOG_TRACE(
            "App ID: {}, DLC ID: {}, Status: {}, Original: {}, Is Unlocked: {}",
            app_id_str, dlc_id_str, Json(status).dump(), original_function(), is_unlocked
        )

        return is_unlocked;
    }

    Vector<DLC> get_extra_dlcs(AppId_t app_id) {
        return DLC::get_dlcs_from_apps(instance.extra_dlcs, app_id);
    }
}
