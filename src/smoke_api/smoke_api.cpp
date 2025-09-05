#include <koalabox/config.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/globals.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/path.hpp>
#include <koalabox/paths.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win.hpp>

#include "build_config.h"

#include "smoke_api.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/steamclient/steamclient.hpp"

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

namespace {
    namespace kb = koalabox;

    HMODULE original_steamapi_handle = nullptr;

    void start_dll_listener() {
        kb::dll_monitor::init_listener(
            {STEAMCLIENT_DLL},
            [&](const HMODULE& module_handle, auto&) {
                KB_HOOK_DETOUR_MODULE(CreateInterface, module_handle);
            }
        );
    }
}

namespace smoke_api {
    void init(const HMODULE module_handle) {
        try {
            kb::globals::init_globals(module_handle, PROJECT_NAME);

            config::instance = kb::config::parse<config::Config>();

            if(config::instance.logging) {
                kb::logger::init_file_logger(kb::paths::get_log_path());
            }

            LOG_INFO("{} v{} | Built at '{}'", PROJECT_NAME, PROJECT_VERSION, __TIMESTAMP__);
            LOG_DEBUG("Parsed config:\n{}", nlohmann::ordered_json(config::instance).dump(2));

            const auto exe_path = kb::win::get_module_path(nullptr);
            const auto exe_name = kb::path::to_str(exe_path.filename());

            LOG_DEBUG("Process name: '{}' [{}-bit]", exe_name, kb::util::BITNESS);

            // We need to hook functions in either mode
            kb::hook::init(true);

            if(kb::hook::is_hook_mode(module_handle, STEAMAPI_DLL)) {
                LOG_INFO("Detected hook mode");

                start_dll_listener();
            } else {
                LOG_INFO("Detected proxy mode");

                const auto self_path = kb::paths::get_self_dir();
                original_steamapi_handle = kb::loader::load_original_library(
                    self_path,
                    STEAMAPI_DLL
                );
                start_dll_listener();
            }

            LOG_INFO("Initialization complete");
        } catch(const std::exception& e) {
            kb::util::panic(fmt::format("Initialization error: {}", e.what()));
        }
    }

    void shutdown() {
        try {
            if(original_steamapi_handle != nullptr) {
                kb::win::free_library(original_steamapi_handle);
                original_steamapi_handle = nullptr;
            }

            // TODO: Unhook everything

            LOG_INFO("Shutdown complete");
        } catch(const std::exception& e) {
            const auto msg = std::format("Shutdown error: {}", e.what());
            LOG_ERROR(msg);
        }

        kb::logger::shutdown();
    }

    AppId_t get_app_id() {
        try {
            const auto app_id_str = kb::win::get_env_var("SteamAppId");
            static auto app_id = std::stoi(app_id_str);
            return app_id;
        } catch(const std::exception& e) {
            LOG_ERROR("Failed to get app id: {}", e.what());
            return 0;
        }
    }
}
