#include <koalabox/http_client.hpp>
#include <koalabox/logger.hpp>

#include "smoke_api/api.hpp"
#include "smoke_api/types.hpp"

namespace api {
    struct SteamResponse {
        uint32_t success = 0;
        std::vector<DLC> dlcs;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            SteamResponse,
            success,
            dlcs
        ) // NOLINT(misc-const-correctness)
    };

    std::optional<std::vector<DLC>> fetch_dlcs_from_github(AppId_t app_id) noexcept {
        try {
            constexpr auto url = "https://raw.githubusercontent.com/"
                "acidicoala/public-entitlements/main/steam/v2/dlc.json";
            const auto json = koalabox::http_client::get_json(url);
            const auto response = json.get<AppDlcNameMap>();

            return DLC::get_dlcs_from_apps(response, app_id);
        } catch(const nlohmann::json::exception& e) {
            LOG_ERROR("Failed to fetch DLC list from GitHub: {}", e.what());
            return std::nullopt;
        }
    }

    std::optional<std::vector<DLC>> fetch_dlcs_from_steam(AppId_t app_id) noexcept {
        try {
            // TODO: Communicate directly with Steam servers.
            // ref.: https://github.com/SteamRE/SteamKit
            const auto url =
                fmt::format("https://store.steampowered.com/dlc/{}/ajaxgetdlclist", app_id);
            const auto json = koalabox::http_client::get_json(url);

            const auto [success, dlcs] = json.get<SteamResponse>();

            if(success != 1) {
                throw std::runtime_error("Web API responded with 'success' != 1");
            }

            return dlcs;
        } catch(const std::exception& e) {
            LOG_ERROR("Failed to fetch dlc list from Steam: {}", e.what());
            return std::nullopt;
        }
    }
}