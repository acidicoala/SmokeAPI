#include <smoke_api/smoke_api.hpp>
#include <build_config.h>
#include <core/config.hpp>
#include <core/globals.hpp>
#include <core/paths.hpp>
#include <steam_functions/steam_functions.hpp>
#include <koalabox/config_parser.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/cache.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/win_util.hpp>

// TODO: Define COMPILE_KOALAGEDDON in CMake
#ifndef _WIN64
#include <koalageddon/koalageddon.hpp>
#endif

void init_proxy_mode() {
    LOG_INFO("🔀 Detected proxy mode")

    globals::steamapi_module = koalabox::loader::load_original_library(paths::get_self_path(), STEAMAPI_DLL);
}

void init_hook_mode() {
    LOG_INFO("🪝 Detected hook mode")

    koalabox::dll_monitor::init_listener(
        STEAMCLIENT_DLL, [](const HMODULE& library) {
            globals::steamclient_module = library;

            DETOUR_STEAMCLIENT(CreateInterface)

            koalabox::dll_monitor::shutdown_listener();
        }
    );

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
}

bool is_valve_steam(const String& exe_name) {
    if (not koalabox::util::strings_are_equal(exe_name, "steam.exe")) {
        return false;
    }

    const HMODULE steam_handle = koalabox::win_util::get_module_handle_or_throw(nullptr);
    const auto manifest = koalabox::win_util::get_module_manifest(steam_handle);

    // Verify that it's steam from valve, and not some other executable coincidentally named steam

    if (!manifest) {
        // Steam.exe is expected to have a manifest
        return false;
    }

    // Steam.exe manifest is expected to contain this string
    return manifest->find("valvesoftware.steam.steam") != String::npos;
}

namespace smoke_api {

    void init(HMODULE module_handle) {
        try {
            DisableThreadLibraryCalls(module_handle);

            globals::smokeapi_handle = module_handle;

            koalabox::cache::init_cache(paths::get_cache_path());

            config::init();

            if (config::instance.logging) {
                koalabox::logger::init_file_logger(paths::get_log_path());
            }

            LOG_INFO("🐨 {} v{}", PROJECT_NAME, PROJECT_VERSION)

            const auto exe_path = Path(koalabox::win_util::get_module_file_name_or_throw(nullptr));
            const auto exe_name = exe_path.filename().string();

            LOG_DEBUG(R"(Process name: "{}" [{}-bit])", exe_name, BITNESS)

            if (koalabox::hook::is_hook_mode(globals::smokeapi_handle, STEAMAPI_DLL)) {
                koalabox::hook::init(true);

                if (is_valve_steam(exe_name)) {
#ifndef _WIN64
                    LOG_INFO("🐨💥 Detected Koalageddon mode")
                    koalageddon::init();
#endif
                } else {
                    init_hook_mode();
                }
            } else {
                init_proxy_mode();
            }
            LOG_INFO("🚀 Initialization complete")
        } catch (const Exception& ex) {
            koalabox::util::panic(fmt::format("Initialization error: {}", ex.what()));
        }
    }

    void shutdown() {
        try {
            if (globals::steamapi_module != nullptr) {
                koalabox::win_util::free_library(globals::steamapi_module);
            }

            LOG_INFO("💀 Shutdown complete")
        } catch (const Exception& ex) {
            LOG_ERROR("Shutdown error: {}", ex.what())
        }
    }
}
