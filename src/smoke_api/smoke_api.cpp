#include <smoke_api/smoke_api.hpp>
#include <core/paths.hpp>
#include <steam_functions/steam_functions.hpp>
#include <build_config.h>

#include <koalabox/config_parser.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/win_util.hpp>
#include <core/globals.hpp>

#ifndef _WIN64
#include <koalageddon/koalageddon.hpp>
#endif

namespace smoke_api {
    Config config = {}; // NOLINT(cert-err58-cpp)

    HMODULE original_library = nullptr;

    HMODULE self_module = nullptr;

    bool is_hook_mode = false;

    void init_proxy_mode() {
        logger->info("🔀 Detected proxy mode");

        original_library = loader::load_original_library(paths::get_self_path(), ORIGINAL_DLL);
    }

    void init_hook_mode() {
        logger->info("🪝 Detected hook mode");

        dll_monitor::init(STEAMCLIENT_DLL, [](const HMODULE& library) {
            original_library = library;

            DETOUR_ORIGINAL(CreateInterface)
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

    void init(HMODULE module_handle) {
        try {
            DisableThreadLibraryCalls(module_handle);

            globals::self_module = module_handle;

            koalabox::project_name = PROJECT_NAME;

            config = config_parser::parse<Config>(paths::get_config_path());

            const auto exe_path = Path(win_util::get_module_file_name_or_throw(nullptr));
            const auto exe_name = exe_path.filename().string();
            const auto exe_bitness = util::is_x64() ? 64 : 32;

            if (config.logging) {
                logger = file_logger::create(paths::get_log_path());
            }

            logger->info("🐨 {} v{}", PROJECT_NAME, PROJECT_VERSION);

            logger->debug(R"(Process name: "{}" [{}-bit])", exe_name, exe_bitness);

            is_hook_mode = hook::is_hook_mode(self_module, ORIGINAL_DLL);

            if (is_hook_mode) {
                hook::init(true);

#ifdef _WIN64
                init_hook_mode();
#else
                // TODO: Check if it's steam from valve
                if (util::strings_are_equal(exe_name, "steam.exe")) {
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
            if (is_hook_mode) {
                dll_monitor::shutdown();
            } else {
                win_util::free_library(original_library);
            }

            logger->info("💀 Shutdown complete");
        } catch (const Exception& ex) {
            logger->error("Shutdown error: {}", ex.what());
        }
    }

    // FIXME: Support for app_id for koalageddon mode
    bool should_unlock(uint32_t app_id) {
        return config.unlock_all != config.override.contains(app_id);
    }

}
