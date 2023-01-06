#include <core/cache.hpp>
#include <core/paths.hpp>
#include <koalabox/io.hpp>

namespace cache {
    Cache read_cache_from_disk() {
        try {
            const auto cache_string = io::read_file(paths::get_cache_path());

            if (cache_string.empty()) {
                return {};
            }

            return nlohmann::json::parse(cache_string).get<Cache>();
        } catch (const Exception& e) {
            logger->warn("{} -> Failed to read cache from disk: {}", __func__, e.what());

            return {};
        }
    }

    void write_cache_to_disk(const Cache& cache) {
        try {
            const auto cache_string = nlohmann::json(cache).dump(2);

            io::write_file(paths::get_cache_path(), cache_string);
        } catch (const Exception& e) {
            logger->error("{} -> Failed to write cache to disk: {}", __func__, e.what());
        }
    }

    Vector<AppId_t> get_dlc_ids(AppId_t app_id) {
        const auto cache = read_cache_from_disk();

        const auto app_id_str = std::to_string(app_id);

        if (cache.apps.contains(app_id_str)) {
            return cache.apps.at(app_id_str).dlc_ids;
        }

        return {};
    }

    std::optional<koalageddon::KoalageddonConfig> get_koalageddon_config() {
        const auto cache = read_cache_from_disk();

        if (cache.koalageddon_config.is_null()) {
            return std::nullopt;
        }

        return cache.koalageddon_config.get<koalageddon::KoalageddonConfig>();
    }

    void save_dlc_ids(AppId_t app_id, const Vector<AppId_t>& dlc_ids) {
        logger->debug("{} -> Caching DLC IDs for the app: {}", __func__, app_id);

        auto cache = read_cache_from_disk();

        const auto app_id_str = std::to_string(app_id);

        if (not cache.apps.contains(app_id_str)) {
            cache.apps[app_id_str] = {};
        }

        cache.apps[app_id_str].dlc_ids = dlc_ids;

        write_cache_to_disk(cache);
    }

    void save_koalageddon_config(const koalageddon::KoalageddonConfig& config) {
        logger->debug("{} -> Caching koalageddon config", __func__);

        auto cache = read_cache_from_disk();

        cache.koalageddon_config = config;

        write_cache_to_disk(cache);
    }
}
