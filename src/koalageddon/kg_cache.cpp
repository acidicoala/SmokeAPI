#include <koalageddon/kg_cache.hpp>
#include <koalabox/cache.hpp>
#include <koalabox/logger.hpp>

constexpr auto KEY_KG_CONFIG = "koalageddon_config";

namespace koalageddon::kg_cache {

    std::optional<KoalageddonConfig> get_koalageddon_config() {
        try {
            const auto cache = koalabox::cache::read_from_cache(KEY_KG_CONFIG);

            return cache[KEY_KG_CONFIG].get<KoalageddonConfig>();
        } catch (const Exception& e) {
            LOG_ERROR("Failed to get cached koalageddon config: {}", e.what())

            return std::nullopt;
        }
    }

    bool save_koalageddon_config(const KoalageddonConfig& config) {
        try {
            LOG_DEBUG("Caching koalageddon config")

            return koalabox::cache::save_to_cache(KEY_KG_CONFIG, Json(config));
        } catch (const Exception& e) {
            LOG_ERROR("Failed to cache koalageddon config: {}", e.what())

            return false;
        }
    }

}
