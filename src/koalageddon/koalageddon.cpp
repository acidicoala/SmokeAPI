#include <koalageddon/koalageddon.hpp>
#include <koalageddon/vstdlib.hpp>
#include <koalageddon/cache.hpp>
#include <build_config.h>
#include <core/config.hpp>
#include <steam_functions/steam_functions.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/http_client.hpp>
#include <koalabox/util.hpp>
#include <koalabox/logger.hpp>

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
                LOG_ERROR("Failed to get local koalageddon config: {}", ex.what())
            }
        }

        try {
            // Then try to fetch config from GitHub
            const String url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/koalageddon/v2/steam.json";
            config = koalabox::http_client::fetch_json(url).get<decltype(config)>();

            cache::save_koalageddon_config(config);

            return "GitHub repository";
        } catch (const Exception& ex) {
            LOG_ERROR("Failed to get remote koalageddon config: {}", ex.what())
        }

        try {
            // Then try to get a cached copy of a previously fetched config.
            // We expect this unboxing to throw exception if no koalageddon config is present.
            config = cache::get_koalageddon_config().value();

            return "disk cache";
        } catch (const Exception& ex) {
            LOG_ERROR("Failed to get cached koalageddon config: {}", ex.what())
        }

        // Finally, fallback on the default config
        config = {};
        return "default config bundled in the binary";
    }

    void init() {
        std::thread(
            []() {
                const auto kg_config_source = init_koalageddon_config();
                LOG_INFO("Loaded Koalageddon config from the {}", kg_config_source)
            }
        ).detach();

        koalabox::dll_monitor::init_listener(
            {VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& module_handle, const String& name) {
                try {
                    if (koalabox::util::strings_are_equal(name, VSTDLIB_DLL)) {
                        // VStdLib DLL handles Family Sharing functions

                        globals::vstdlib_module = module_handle;

                        if (config::instance.unlock_family_sharing) {
                            DETOUR_VSTDLIB(Coroutine_Create)
                        }
                    } else if (koalabox::util::strings_are_equal(name, STEAMCLIENT_DLL)) {
                        // SteamClient DLL handles unlocking functions

                        globals::steamclient_module = module_handle;

                        DETOUR_STEAMCLIENT(CreateInterface)
                    }

                    if (globals::vstdlib_module != nullptr && globals::steamclient_module != nullptr) {
                        koalabox::dll_monitor::shutdown_listener();
                    }
                } catch (const Exception& ex) {
                    LOG_ERROR(
                        "Koalageddon mode dll monitor process_interface_selector error. Module: '{}', Message: {}",
                        name, ex.what())
                }
            }
        );
    }
}
