#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>
#include <build_config.h>

#include <koalabox/config_parser.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/win_util.hpp>
#ifndef _WIN64
#include <koalabox/patcher.hpp>
#endif

#define DETOUR_EX(FUNC, ADDRESS) hook::detour_or_warn(ADDRESS, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));
#define DETOUR(FUNC) hook::detour_or_warn(original_library, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));

namespace smoke_api {
    Config config = {}; // NOLINT(cert-err58-cpp)

    KoalageddonConfig koalageddon_config = {};

    HMODULE original_library = nullptr;

    bool is_hook_mode = false;

    Path self_directory;

    void init_config() {
        // TODO: Detect koalageddon mode first, and then fetch config from corresponding directory
        config = config_parser::parse<Config>(self_directory / PROJECT_NAME".json");
    }

    /**
     * @return A string representing the source of the config.
     */
    String init_koalageddon_config() {
        try {
            // First try to read a local config override
            koalageddon_config = config.koalageddon_config.get<KoalageddonConfig>();
            return "local config override";
        } catch (const Exception& ex) {
            logger->debug("Local config parse exception: {}", ex.what());
        }

        // TODO: Remote source with local cache

        // Finally, fallback on the default config
        return "default config bundled in the binary";
    }

    void init_koalageddon_mode() {
#ifndef _WIN64
        logger->info("🐨 Detected Koalageddon mode 💥");

        const auto kg_config_source = init_koalageddon_config();
        logger->info("Loaded Koalageddon config from the {}", kg_config_source);

        dll_monitor::init({VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& library, const String& name) {
            original_library = library; // TODO: Is this necessary?

            if (name == VSTDLIB_DLL) {
                // Family Sharing functions
                DETOUR(Coroutine_Create)
            } else if (name == STEAMCLIENT_DLL) {
                // Unlocking functions
                auto interface_interceptor_address = (FunctionAddress) patcher::find_pattern_address(
                    win_util::get_module_info(library),
                    "SteamClient_Interface_Interceptor",
                    koalageddon_config.interface_interceptor_pattern
                );

                if (interface_interceptor_address) {
                    DETOUR_EX(SteamClient_Interface_Interceptor, interface_interceptor_address)
                }
            }
        });
#endif
    }

    void init_proxy_mode() {
        logger->info("🔀 Detected proxy mode");

        original_library = loader::load_original_library(self_directory, ORIGINAL_DLL);
    }

    void init_hook_mode() {
        logger->info("🪝 Detected hook mode");

        dll_monitor::init(STEAMCLIENT_DLL, [](const HMODULE& library) {
            original_library = library;

            DETOUR(CreateInterface)
        });

        // Hooking steam_api has show itself to be less desirable than steamclient
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

    void init(HMODULE self_module) {
        try {
            DisableThreadLibraryCalls(self_module);

            koalabox::project_name = PROJECT_NAME;

            self_directory = loader::get_module_dir(self_module);

            init_config();

            const auto exe_path = Path(win_util::get_module_file_name_or_throw(nullptr));
            const auto exe_name = exe_path.filename().string();
            const auto exe_bitness = util::is_x64() ? 64 : 32;

            if (config.logging) {
                logger = file_logger::create(self_directory / fmt::format("{}.log", PROJECT_NAME));
            }

            logger->info("🐨 {} v{}", PROJECT_NAME, PROJECT_VERSION);

            logger->debug(R"(Process name: "{}" [{}-bit])", exe_name, exe_bitness);

            is_hook_mode = hook::is_hook_mode(self_module, ORIGINAL_DLL);

            if (is_hook_mode) {
                hook::init(true);

                if (util::strings_are_equal(exe_name, "steam.exe") && !util::is_x64()) {
                    init_koalageddon_mode();
                } else {
                    init_hook_mode();
                }
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

    bool should_unlock(uint32_t app_id) {
        return config.unlock_all != config.override.contains(app_id);
    }

}
