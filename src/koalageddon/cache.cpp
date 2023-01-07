#include <koalageddon/cache.hpp>
#include <koalabox/cache.hpp>
#include <koalabox/logger.hpp>

constexpr auto KEY_KG_CONFIG = "koalageddon_config";

namespace koalageddon::cache {

    std::optional<KoalageddonConfig> get_koalageddon_config() {
        try {
            const auto cache = koalabox::cache::read_from_cache(KEY_KG_CONFIG).value();

            return cache.at(KEY_KG_CONFIG).get<KoalageddonConfig>();
        } catch (const Exception& e) {
            LOG_ERROR("{} -> Failed to get cached koalageddon config: {}", __func__, e.what())

            return std::nullopt;
        }
    }

    bool save_koalageddon_config(const KoalageddonConfig& config) {
        try {
            LOG_DEBUG("{} -> Caching koalageddon config", __func__)

            return koalabox::cache::save_to_cache(KEY_KG_CONFIG, config);
        } catch (const Exception& e) {
            LOG_ERROR("{} -> Failed to cache koalageddon config: {}", __func__, e.what())

            return false;
        }
    }

}
