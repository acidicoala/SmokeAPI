#include <koalageddon/koalageddon.hpp>
#include <koalageddon/vstdlib.hpp>
#include <koalageddon/kg_cache.hpp>
#include <build_config.h>
#include <smoke_api/config.hpp>
#include <steam_api_exports/steam_api_exports.hpp>
#include <core/api.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/win_util.hpp>

namespace koalageddon {

    KoalageddonConfig config; // NOLINT(cert-err58-cpp)

    /**
    * @return A string representing the source of the config.
    */
    void init_koalageddon_config() {
        const auto print_source = [](const String& source) {
            LOG_INFO("Loaded Koalageddon config from the {}", source)
        };

        // First try to read a local config override
        const auto& kg_config = smoke_api::config::instance.koalageddon_config;
        if (!kg_config.is_null()) {
            try {
                config = kg_config.get<decltype(config)>();

                print_source("local config override");
                return;
            } catch (const Exception& ex) {
                LOG_ERROR("Failed to get local koalageddon config: {}", ex.what())
            }
        }

        // Then try to get a cached copy of a previously fetched config.
        try {
            config = kg_cache::get_koalageddon_config().value();

            print_source("disk cache");
        } catch (const Exception& ex) {
            LOG_ERROR("Failed to get cached koalageddon config: {}", ex.what())

            print_source("default config bundled in the binary");

            // Fallback on the default config, to continue execution immediately.
            config = {};
        }

        // Finally, fetch the remote config from GitHub, and inform user about the need to restart Steam,
        // if a new config has been fetched
        NEW_THREAD({
            try {
                const auto github_config_opt = api::fetch_koalageddon_config();
                if (!github_config_opt) {
                    return;
                }

                const auto github_config = *github_config_opt;

                kg_cache::save_koalageddon_config(github_config);

                if (github_config == config) {
                    LOG_DEBUG("Fetched Koalageddon config is equal to existing config")

                    return;
                }

                LOG_DEBUG("Fetched a new Koalageddon config")

                ::MessageBox(
                    nullptr,
                    TEXT(
                        "SmokeAPI has downloaded an updated config for Koalageddon mode. "
                        "Please restart Steam in order to apply the new Koalageddon config. "
                    ),
                    TEXT("SmokeAPI - Koalageddon"),
                    MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK
                );
            } catch (const Exception& ex) {
                LOG_ERROR("Failed to get remote koalageddon config: {}", ex.what())
            }
        })
    }

    void init() {
        init_koalageddon_config();

        koalabox::dll_monitor::init_listener(
            {VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& module_handle, const String& name) {
                try {
                    if (name < equals > VSTDLIB_DLL) {
                        // VStdLib DLL handles Family Sharing functions

                        globals::vstdlib_module = module_handle;

                        if (smoke_api::config::instance.unlock_family_sharing) {
                            DETOUR_VSTDLIB(Coroutine_Create)
                        }
                    } else if (name < equals > STEAMCLIENT_DLL) {
                        // SteamClient DLL handles unlocking functions

                        globals::steamclient_module = module_handle;

                        DETOUR_STEAMCLIENT(CreateInterface)
                    }

                    if (globals::vstdlib_module != nullptr && globals::steamclient_module != nullptr) {
                        koalabox::dll_monitor::shutdown_listener();
                    }
                } catch (const Exception& ex) {
                    LOG_ERROR(
                        "Error listening to DLL load events. Module: '{}', Message: {}",
                        name, ex.what()
                    )
                }
            }
        );
    }
}
