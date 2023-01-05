#include <build_config.h>
#include <koalageddon/koalageddon.hpp>
#include <smoke_api/smoke_api.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/http_client.hpp>
#include <koalabox/io.hpp>

namespace koalageddon {
    KoalageddonConfig config = {};

    /**
    * @return A string representing the source of the config.
    */
    String init_koalageddon_config() {
        if (!smoke_api::config.koalageddon_config.is_null()) {
            try {
                // First try to read a local config override
                config = smoke_api::config.koalageddon_config.get<decltype(config)>();

                return "local config override";
            } catch (const Exception& ex) {
                logger->error("Local koalageddon config parse exception: {}", ex.what());
            }
        }

        const auto config_cache_path = smoke_api::self_directory / "SmokeAPI.koalageddon.json";

        try {
            // Then try to fetch config from GitHub
            const String url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/koalageddon/v2/steam.json";
            config = http_client::fetch_json(url).get<decltype(config)>();

            io::write_file(config_cache_path, nlohmann::json(config).dump(2));

            return "GitHub repository";
        } catch (const Exception& ex) {
            logger->error("Remote koalageddon config parse exception: {}", ex.what());
        }

        try {
            // Then try to get a cached copy of a previously fetched config
            const auto cache = io::read_file(config_cache_path);

            config = nlohmann::json::parse(cache).get<decltype(config)>();

            return "Local cache";
        } catch (const Exception& ex) {
            logger->error("Cached koalageddon config parse exception: {}", ex.what());
        }

        // Finally, fallback on the default config
        return "default config bundled in the binary";
    }

    void init() {
#ifndef _WIN64
        logger->info("🐨 Detected Koalageddon mode 💥");

        std::thread([]() {
            const auto kg_config_source = init_koalageddon_config();
            logger->info("Loaded Koalageddon config from the {}", kg_config_source);
        }).detach();

        dll_monitor::init({VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& library, const String& name) {
            try {
                smoke_api::original_library = library;

                static auto init_count = 0;
                if (name == VSTDLIB_DLL) {
                    // VStdLib DLL handles Family Sharing functions
                    if (smoke_api::config.unlock_family_sharing) {
                        init_vstdlib_hooks();
                    }
                    init_count++;
                } else if (name == STEAMCLIENT_DLL) {
                    // SteamClient DLL handles unlocking functions
                    init_steamclient_hooks();
                    init_count++;
                }

                if (init_count == 2) {
                    dll_monitor::shutdown();
                }
            } catch (const Exception& ex) {
                logger->error("Koalageddon mode dll monitor init error. Module: '{}', Message: {}", name, ex.what());
            }
        });
#endif
    }
}
