#include <koalageddon/koalageddon.hpp>
#include <build_config.h>
#include <core/cache.hpp>
#include <core/config.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/http_client.hpp>
#include <koalabox/util.hpp>
#include <steam_functions/steam_functions.hpp>

namespace koalageddon {
    KoalageddonConfig config; // NOLINT(cert-err58-cpp)

    /**
    * @return A string representing the source of the config.
    */
    String init_koalageddon_config() {
        if (!config::instance.koalageddon_config.is_null()) {
            try {
                // First try to read a local config override
                config = config::instance.koalageddon_config.get<decltype(config)>();

                return "local config override";
            } catch (const Exception& ex) {
                logger->error("Failed to get local koalageddon config: {}", ex.what());
            }
        }

        try {
            // Then try to fetch config from GitHub
            const String url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/koalageddon/v2/steam.json";
            config = http_client::fetch_json(url).get<decltype(config)>();

            cache::save_koalageddon_config(config);

            return "GitHub repository";
        } catch (const Exception& ex) {
            logger->error("Failed to get remote koalageddon config: {}", ex.what());
        }

        try {
            // Then try to get a cached copy of a previously fetched config.
            // We expect this unboxing to throw exception if no koalageddon config is present.
            config = *cache::get_koalageddon_config();

            return "disk cache";
        } catch (const Exception& ex) {
            logger->error("Failed to get cached koalageddon config: {}", ex.what());
        }

        // Finally, fallback on the default config
        config = {};
        return "default config bundled in the binary";
    }

    void init() {
        std::thread([]() {
            const auto kg_config_source = init_koalageddon_config();
            logger->info("Loaded Koalageddon config from the {}", kg_config_source);
        }).detach();

        dll_monitor::init({VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& module_handle, const String& name) {
            try {
                if (util::strings_are_equal(name, VSTDLIB_DLL)) {
                    // VStdLib DLL handles Family Sharing functions

                    globals::vstdlib_module = module_handle;

                    if (config::instance.unlock_family_sharing) {
                        DETOUR_VSTDLIB(Coroutine_Create)
                    }
                } else if (util::strings_are_equal(name, STEAMCLIENT_DLL)) {
                    // SteamClient DLL handles unlocking functions

                    globals::steamclient_module = module_handle;

                    DETOUR_STEAMCLIENT(CreateInterface)
                }

                if (globals::vstdlib_module != nullptr && globals::steamclient_module != nullptr) {
                    dll_monitor::shutdown();
                }
            } catch (const Exception& ex) {
                logger->error("Koalageddon mode dll monitor init error. Module: '{}', Message: {}", name, ex.what());
            }
        });
    }
}
