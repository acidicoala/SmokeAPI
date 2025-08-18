#include <store_mode/store_api.hpp>
#include <koalabox/http_client.hpp>

namespace store::api {

    std::optional<StoreConfig> fetch_store_config() noexcept {
        try {
            const String url =
                "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/steam/v2/store_config.json";
            const auto kg_config_json = koalabox::http_client::get_json(url);

            return kg_config_json.get<StoreConfig>();
        } catch (const Exception& e) {
            LOG_ERROR("Failed to fetch Store config from GitHub: {}", e.what());
            return std::nullopt;
        }
    }

}