#include <core/api.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/http_client.hpp>

namespace api {

    struct SteamResponse {
        uint32_t success = 0;
        Vector<DLC> dlcs;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SteamResponse, success, dlcs) // NOLINT(misc-const-correctness)
    };

    std::optional<Vector<DLC>> fetch_dlcs_from_github(AppId_t app_id) noexcept {
        try {
            const auto* url =
                "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/steam/v2/dlc.json";
            const auto json = koalabox::http_client::fetch_json(url);
            const auto response = json.get<AppDlcNameMap>();

            return DLC::get_dlcs_from_apps(response, app_id);
        } catch (const Json::exception& e) {
            LOG_ERROR("Failed to fetch dlc list from GitHub: {}", e.what())
            return std::nullopt;
        }
    }

    std::optional<Vector<DLC>> fetch_dlcs_from_steam(AppId_t app_id) noexcept {
        try {
            // TODO: Communicate directly with Steam servers.
            // ref.: https://github.com/SteamRE/SteamKit
            const auto url = fmt::format("https://store.steampowered.com/dlc/{}/ajaxgetdlclist", app_id);
            const auto json = koalabox::http_client::fetch_json(url);

            const auto response = json.get<SteamResponse>();

            if (response.success != 1) {
                throw std::runtime_error("Web API responded with 'success' != 1");
            }

            return response.dlcs;
        } catch (const Exception& e) {
            LOG_ERROR("Failed to fetch dlc list from Steam: {}", e.what())
            return std::nullopt;
        }
    }

}
