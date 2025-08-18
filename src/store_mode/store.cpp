#include "koalabox/str.hpp"

#include <build_config.h>
#include <common/steamclient_exports.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/ipc.hpp>
#include <koalabox/logger.hpp>
#include <smoke_api/config.hpp>
#include <store_mode/store.hpp>
#include <store_mode/store_api.hpp>
#include <store_mode/store_cache.hpp>
#include <store_mode/vstdlib/vstdlib.hpp>

namespace store {

    StoreConfig config; // NOLINT(cert-err58-cpp)

    /**
    * @return A string representing the source of the config.
    */
    void init_store_config() {
        const auto print_source = [](const String& source) {
            LOG_INFO("Loaded Store config from the {}", source);
        };

        // First try to read a local config override
        const auto& kg_config = smoke_api::config::instance.store_config;
        if (!kg_config.is_null()) {
            try {
                config = kg_config.get<decltype(config)>();

                print_source("local config override");
                return;
            } catch (const Exception& ex) {
                LOG_ERROR("Failed to get local store_mode config: {}", ex.what());
            }
        }

        // Then try to get a cached copy of a previously fetched config.
        try {
            config = store_cache::get_store_config().value();

            print_source("disk cache");
        } catch (const Exception& ex) {
            LOG_ERROR("Failed to get cached store_mode config: {}", ex.what());

            print_source("default config bundled in the binary");

            // Fallback on the default config, to continue execution immediately.
            config = {};
        }

        // Finally, fetch the remote config from GitHub, and inform user about the need to restart Steam,
        // if a new config has been fetched
        NEW_THREAD({
            try {
                const auto github_config_opt = api::fetch_store_config();
                if (!github_config_opt) {
                    return;
                }

                const auto github_config = *github_config_opt;

                store_cache::save_store_config(github_config);

                if (github_config == config) {
                    LOG_DEBUG("Fetched Store config is equal to existing config");

                    return;
                }

                LOG_DEBUG("Fetched a new Store config")

                ::MessageBox(
                    nullptr,
                    TEXT(
                        "SmokeAPI has downloaded an updated config for Store mode. "
                        "Please restart Steam in order to apply the new Store config. "
                    ),
                    TEXT("SmokeAPI - Store"),
                    MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK
                );
            } catch (const Exception& ex) {
                LOG_ERROR("Failed to get remote store_mode config: {}", ex.what());
            }
        })
    }

    void init_store_mode() {
        init_store_config();

        koalabox::dll_monitor::init_listener(
            {VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& module_handle, const String& name) {
                try {
                    if (koalabox::str::eq(name, VSTDLIB_DLL)) {
                        // VStdLib DLL handles Family Sharing functions

                        globals::vstdlib_module = module_handle;

                        if (smoke_api::config::instance.unlock_family_sharing) {
                            DETOUR_VSTDLIB(Coroutine_Create)
                        }
                    } else if (koalabox::str::eq(name, STEAMCLIENT_DLL)) {
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
                    );
                }
            }
        );

        NEW_THREAD({
            koalabox::ipc::init_pipe_server("smokeapi.store.steam", [](const koalabox::ipc::Request& request) {
                koalabox::ipc::Response response;

                if (request.name < equals > "config::reload") {
                    smoke_api::config::ReloadConfig();
                    response.success = true;
                } else {
                    response.success = false;
                    response.data["error_message"] = "Invalid request name: " + request.name;
                }

                return response;
            });
        })
    }
}
