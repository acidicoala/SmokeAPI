#include <build_config.h>
#include <koalageddon/koalageddon.hpp>
#include <smoke_api/smoke_api.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/win_util.hpp>
#include <koalabox/patcher.hpp>
#include <core/macros.hpp>

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
            logger->debug("Local config parse exception: {}", ex.what());
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
            smoke_api::original_library = library;

            if (name == VSTDLIB_DLL) {
                // Family Sharing functions
                DETOUR_ORIGINAL(Coroutine_Create)
            } else if (name == STEAMCLIENT_DLL) {
                // Unlocking functions
                auto interface_interceptor_address = (FunctionAddress) patcher::find_pattern_address(
                    win_util::get_module_info(library),
                    "SteamClient_Interface_Interceptor",
                    config.interface_interceptor_pattern
                );

                if (interface_interceptor_address) {
                    DETOUR(SteamClient_Interface_Interceptor, interface_interceptor_address)
                }
            }
        });
#endif
    }
}
