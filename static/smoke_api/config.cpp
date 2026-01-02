#include <koalabox/config.hpp>
#include <koalabox/logger.hpp>

#include "smoke_api/config.hpp"

namespace smoke_api::config {
    Config& get() noexcept {
        static Config config;
        return config;
    }

    std::vector<DLC> get_extra_dlcs(const uint32_t app_id) {
        return DLC::get_dlcs_from_apps(get().extra_dlcs, app_id);
    }

    bool is_dlc_unlocked(
        const AppId_t app_id,
        const AppId_t dlc_id,
        const std::function<bool()>& original_function
    ) {
        auto status = get().default_app_status;

        const auto app_id_str = std::to_string(app_id);
        if(get().override_app_status.contains(app_id_str)) {
            status = get().override_app_status[app_id_str];
        }

        const auto dlc_id_str = std::to_string(dlc_id);
        if(get().override_dlc_status.contains(dlc_id_str)) {
            status = get().override_dlc_status[dlc_id_str];
        }

        bool is_unlocked;

        switch(status) {
        case AppStatus::UNLOCKED:
            is_unlocked = true;
            break;
        case AppStatus::LOCKED:
            is_unlocked = false;
            break;
        case AppStatus::ORIGINAL:
        case AppStatus::UNDEFINED:
        default:
            is_unlocked = original_function();
            break;
        }

        LOG_TRACE(
            "App ID: {}, DLC ID: {}, Status: {}, Original: {}, Unlocked: {}",
            app_id_str,
            dlc_id_str,
            nlohmann::json(status).dump(),
            original_function(),
            is_unlocked
        );

        return is_unlocked;
    }
}
