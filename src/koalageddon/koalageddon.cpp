#include <build_config.h>
#include <koalageddon/koalageddon.hpp>
#include <smoke_api/smoke_api.hpp>
#include <koalabox/dll_monitor.hpp>

namespace koalageddon {
    KoalageddonConfig config = {};

    /**
    * @return A string representing the source of the config.
    */
    String init_koalageddon_config() {

        try {
            // First try to read a local config override
            config = smoke_api::config.koalageddon_config.get<KoalageddonConfig>();
            return "local config override";
        } catch (const Exception& ex) {
            logger->debug("Local koalageddon config parse exception: {}", ex.what());
        }

        // TODO: Remote source with local cache

        // Finally, fallback on the default config
        return "default config bundled in the binary";
    }

    void init() {
#ifndef _WIN64
        logger->info("🐨 Detected Koalageddon mode 💥");

        const auto kg_config_source = init_koalageddon_config();
        logger->info("Loaded Koalageddon config from the {}", kg_config_source);

        dll_monitor::init({VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& library, const String& name) {
            try {
                smoke_api::original_library = library;

                if (name == VSTDLIB_DLL) {
                    // VStdLib DLL handles Family Sharing functions
                    if (smoke_api::config.unlock_family_sharing) {
                        init_vstdlib_hooks();
                    }
                } else if (name == STEAMCLIENT_DLL) {
                    // SteamClient DLL handles unlocking functions
                    init_steamclient_hooks();
                }
            } catch (const Exception& ex) {
                logger->error("Koalageddon mode dll monitor init error. Module: '{}', Message: {}", name, ex.what());
            }
        });
#endif
    }
}
