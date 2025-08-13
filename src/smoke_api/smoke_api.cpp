#include <smoke_api/smoke_api.hpp>
#include <build_config.h>
#include <smoke_api/config.hpp>
#include <core/globals.hpp>
#include <core/paths.hpp>
#include <common/steamclient_exports.hpp>
#include <koalabox/globals.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/win_util.hpp>
#include <koalabox/util.hpp>

#if COMPILE_STORE_MODE
#include <store_mode/store.hpp>
#endif

// Hooking steam_api has shown itself to be less desirable than steamclient
// for the reasons outlined below:
//
// Calling original in flat functions will actually call the hooked functions
// because the original function redirects the execution to a function taken
// from self pointer, which would have been hooked by SteamInternal_*Interface
// functions.
//
// Furthermore, turns out that many flat functions share the same body,
// which looks like the following snippet:
//
//   mov rax, qword ptr ds:[rcx]
//   jmp qword ptr ds:[rax+immediate]
//
// This means that we end up inadvertently hooking unintended functions.
// Given that hooking steam_api has no apparent benefits, but has inherent flaws,
// the support for it has been dropped from this project.


void override_app_id() {
    const auto override_app_id = smoke_api::config::instance.override_app_id;
    if (override_app_id == 0)
        return;

    LOG_DEBUG("Overriding app id to {}", override_app_id);

    SetEnvironmentVariable(
        TEXT("SteamAppId"),
        std::to_wstring(override_app_id).c_str()
    );
}

void init_proxy_mode() {
    LOG_INFO("🔀 Detected proxy mode")

    override_app_id();

    globals::steamapi_module = koalabox::loader::load_original_library(
        paths::get_self_path(),
        STEAMAPI_DLL
    );
}

void init_hook_mode() {
    LOG_INFO("🪝 Detected hook mode")

    override_app_id();

    koalabox::dll_monitor::init_listener(
        STEAMCLIENT_DLL,
        [](const HMODULE &library) {
            globals::steamclient_module = library;

            DETOUR_STEAMCLIENT(CreateInterface)

            koalabox::dll_monitor::shutdown_listener();
        }
    );
}

bool is_valve_steam(const String &exe_name) noexcept {
    try {
        if (exe_name < not_equals > "steam.exe") {
            return false;
        }

        // Verify that it's steam from valve, and not some other executable coincidentally named steam

        const HMODULE steam_handle = koalabox::win_util::get_module_handle_or_throw(nullptr);
        const auto manifest = koalabox::win_util::get_module_manifest(
            steam_handle);

        // Steam.exe manifest is expected to contain this string
        return manifest < contains > "valvesoftware.steam.steam";
    } catch (const Exception &e) {
        LOG_ERROR("{} -> {}", __func__, e.what())

        return false;
    }
}

namespace smoke_api {
    void init(const HMODULE module_handle) {
        // FIXME: IMPORTANT! Non ascii paths in directories will result in init errors
        try {
            DisableThreadLibraryCalls(module_handle);

            koalabox::globals::init_globals(module_handle, PROJECT_NAME);

            globals::smokeapi_handle = module_handle;

            config::init_config();

            if (config::instance.logging) {
                koalabox::logger::init_file_logger();
            }

            // This kind of timestamp is reliable only for CI builds, as it will reflect the compilation
            // time stamp only when this file gets recompiled.
            LOG_INFO(
                "🐨 {} v{} | Compiled at '{}'", PROJECT_NAME,
                PROJECT_VERSION,
                __TIMESTAMP__
            )

            const Path exe_path = koalabox::win_util::get_module_file_name_or_throw(nullptr);
            const auto exe_name = exe_path.filename().string();

            LOG_DEBUG("Process name: '{}' [{}-bit]", exe_name, BITNESS)

            const bool is_hook_mode = koalabox::hook::is_hook_mode(
                globals::smokeapi_handle,
                STEAMAPI_DLL
            );

            if (is_hook_mode) {
                koalabox::hook::init(true);

                if (is_valve_steam(exe_name)) {
#if COMPILE_STORE_MODE
                    LOG_INFO("🛍️ Detected Store mode")
                    store::init_store_mode();
#endif
                } else {
                    init_hook_mode();
                }
            } else {
                init_proxy_mode();
            }

            LOG_INFO("🚀 Initialization complete")
        } catch (const Exception &ex) {
            koalabox::util::panic(
                fmt::format("Initialization error: {}", ex.what())
            );
        }
    }

    void shutdown() {
        try {
            if (globals::steamapi_module != nullptr) {
                koalabox::win_util::free_library(globals::steamapi_module);
            }

            LOG_INFO("💀 Shutdown complete")
        } catch (const Exception &ex) {
            LOG_ERROR("Shutdown error: {}", ex.what())
        }
    }
}