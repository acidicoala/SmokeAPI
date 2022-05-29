#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>
#include <build_config.h>

#include <koalabox/config_parser.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>

#define DETOUR_EX(FUNC, ADDRESS) hook::detour_or_warn(ADDRESS, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));
#define DETOUR(FUNC) hook::detour_or_warn(original_library, #FUNC, reinterpret_cast<FunctionAddress>(FUNC));

namespace smoke_api {
    Config config = {}; // NOLINT(cert-err58-cpp)

    HMODULE original_library = nullptr;

    bool is_hook_mode = false;

    Path self_directory;

    void init(HMODULE self_module) {
        try {
            DisableThreadLibraryCalls(self_module);

            koalabox::project_name = PROJECT_NAME;

            self_directory = loader::get_module_dir(self_module);

            config = config_parser::parse<Config>(self_directory / PROJECT_NAME".json");

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

                if (util::strings_are_equal(exe_name, "steam.exe")) { // target vstdlib_s.dll
                    logger->info("🐨 Detected Koalageddon mode 💥");

                    dll_monitor::init({VSTDLIB_DLL, STEAMCLIENT_DLL}, [](const HMODULE& library, const String& name) {
                        original_library = library; // TODO: Is this necessary?

                        if (name == VSTDLIB_DLL) {
                            // Family Sharing functions
                            DETOUR(Coroutine_Create)
                        } else if (name == STEAMCLIENT_DLL) {
                            // Unlocking functions
                            // TODO: Un-hardcode the pattern
                            const String pattern("55 8B EC 8B ?? ?? ?? ?? ?? 81 EC ?? ?? ?? ?? 53 FF 15");
                            auto Log_Interface_address = (FunctionAddress) patcher::find_pattern_address(
                                win_util::get_module_info(library), "Log_Interface", pattern
                            );
                            if (Log_Interface_address) {
                                DETOUR_EX(Log_Interface, Log_Interface_address)
                            }
                        }
                    });
                } else if (config.hook_steamclient) { // target steamclient(64).dll
                    logger->info("🪝 Detected hook mode for SteamClient");

                    dll_monitor::init(STEAMCLIENT_DLL, [](const HMODULE& library) {
                        original_library = library;

                        DETOUR(CreateInterface)
                    });
                } else { // target steam_api.dll
                    logger->info("🪝 Detected hook mode for Steam_API");

                    dll_monitor::init(ORIGINAL_DLL, [](const HMODULE& library) {
                        original_library = library;

                        DETOUR(SteamInternal_FindOrCreateUserInterface)
                        DETOUR(SteamInternal_CreateInterface)
                        DETOUR(SteamApps)
                        DETOUR(SteamClient)
                        DETOUR(SteamUser)

                        DETOUR(SteamAPI_ISteamApps_BIsSubscribedApp)
                        DETOUR(SteamAPI_ISteamApps_BIsDlcInstalled)
                        DETOUR(SteamAPI_ISteamApps_GetDLCCount)
                        DETOUR(SteamAPI_ISteamApps_BGetDLCDataByIndex)
                        DETOUR(SteamAPI_ISteamClient_GetISteamGenericInterface)

                        DETOUR(SteamAPI_ISteamInventory_GetResultStatus)
                        DETOUR(SteamAPI_ISteamInventory_GetResultItems)
                        DETOUR(SteamAPI_ISteamInventory_GetResultItemProperty)
                        DETOUR(SteamAPI_ISteamInventory_CheckResultSteamID)
                        DETOUR(SteamAPI_ISteamInventory_GetAllItems)
                        DETOUR(SteamAPI_ISteamInventory_GetItemsByID)
                        DETOUR(SteamAPI_ISteamInventory_GetItemDefinitionIDs)
                        DETOUR(SteamAPI_ISteamInventory_GetItemDefinitionProperty)
                    });
                }
            } else {
                logger->info("🔀 Detected proxy mode for Steam_API");

                original_library = loader::load_original_library(self_directory, ORIGINAL_DLL);
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

    bool should_unlock(uint32_t appId) {
        return config.unlock_all != config.override.contains(appId);
    }

}
