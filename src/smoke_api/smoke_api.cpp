#include <smoke_api/smoke_api.hpp>
#include <build_config.h>
#include <core/config.hpp>
#include <core/globals.hpp>
#include <core/paths.hpp>
#include <steam_functions/steam_functions.hpp>
#include <koalabox/config_parser.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/win_util.hpp>

// TODO: Define COMPILE_KOALAGEDDON in CMake
#ifndef _WIN64
#include <koalageddon/koalageddon.hpp>
#endif

namespace smoke_api {
    using namespace koalabox;

    void init_proxy_mode() {
        logger->info("🔀 Detected proxy mode");

        globals::steamapi_module = loader::load_original_library(paths::get_self_path(), STEAMAPI_DLL);
    }

    void init_hook_mode() {
        logger->info("🪝 Detected hook mode");

        dll_monitor::init(STEAMCLIENT_DLL, [](const HMODULE& library) {
            globals::steamclient_module = library;

            DETOUR_STEAMCLIENT(CreateInterface)

            dll_monitor::shutdown();
        });

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
        if (not util::strings_are_equal(exe_name, "steam.exe")) {
            return false;
        }

        const HMODULE steam_handle = win_util::get_module_handle_or_throw(nullptr);
        const auto manifest = win_util::get_module_manifest(steam_handle);

        // Verify that it's steam from valve, and not some other executable coincidentally named steam

        if (!manifest.has_value()) {
            // Steam.exe is expected to have a manifest
            return false;
        }

        // Steam.exe manifest is expected to contain this string
        return manifest.value().find("valvesoftware.steam.steam") != String::npos;
    }

    void init(HMODULE module_handle) {
        try {
            DisableThreadLibraryCalls(module_handle);

            globals::smokeapi_handle = module_handle;

            koalabox::project_name = PROJECT_NAME;

            config::init();

            const auto exe_path = Path(win_util::get_module_file_name_or_throw(nullptr));
            const auto exe_name = exe_path.filename().string();
            const auto exe_bitness = util::is_x64() ? 64 : 32;

            if (config::instance.logging) {
                logger = file_logger::create(paths::get_log_path());
            }

            logger->info("🐨 {} v{}", PROJECT_NAME, PROJECT_VERSION);

            logger->debug(R"(Process name: "{}" [{}-bit])", exe_name, exe_bitness);

            if (hook::is_hook_mode(globals::smokeapi_handle, STEAMAPI_DLL)) {
                hook::init(true);

#ifdef _WIN64
                init_hook_mode();
#else
                // TODO: Check if it's steam from valve
                if (is_valve_steam(exe_name)) {
                    logger->info("🐨💥 Detected Koalageddon mode");

                    koalageddon::init();
                } else {
                    init_hook_mode();
                }
#endif
            } else {
                init_proxy_mode();
            }
            logger->info("🚀 Initialization complete");
        } catch (const Exception& ex) {
            util::panic(fmt::format("Initialization error: {}", ex.what()));
        }
    }

    void shutdown() {
        try {
            if (globals::steamapi_module != nullptr) {
                win_util::free_library(globals::steamapi_module);
            }

            logger->info("💀 Shutdown complete");
        } catch (const Exception& ex) {
            logger->error("Shutdown error: {}", ex.what());
        }
    }
}
