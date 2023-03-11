#include <store_mode/store_cache.hpp>
#include <koalabox/cache.hpp>

constexpr auto KEY_KG_CONFIG = "store_config";

namespace store::store_cache {

    std::optional<StoreConfig> get_store_config() {
        try {
            return koalabox::cache::get(KEY_KG_CONFIG, Json(nullptr)).get<StoreConfig>();
        } catch (const Exception& e) {
            LOG_ERROR("Failed to get cached store_mode config: {}", e.what())

            return std::nullopt;
        }
    }

    bool save_store_config(const StoreConfig& config) {
        try {
            LOG_DEBUG("Caching store_mode config")

            return koalabox::cache::put(KEY_KG_CONFIG, Json(config));
        } catch (const Exception& e) {
            LOG_ERROR("Failed to cache store_mode config: {}", e.what())

            return false;
        }
    }

}
