#include <koalabox/config.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/globals.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/paths.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win_util.hpp>

#include "build_config.h"

#include "exports/steamclient.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/globals.hpp"
#include "smoke_api/smoke_api.hpp"

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
    namespace fs = std::filesystem;

#define DETOUR_STEAMCLIENT(FUNC)                                                                   \
    kb::hook::detour_or_warn(globals::steamclient_module, #FUNC, reinterpret_cast<uintptr_t>(FUNC));

    void override_app_id() {
        const auto override_app_id = smoke_api::config::instance.override_app_id;
        if (override_app_id == 0) {
            return;
        }

        spdlog::default_logger_raw();
        LOG_DEBUG("Overriding app id to {}", override_app_id);

        SetEnvironmentVariable(TEXT("SteamAppId"), std::to_wstring(override_app_id).c_str());
    }

    void init_proxy_mode() {
        LOG_INFO("Detected proxy mode");

        const auto self_path = kb::paths::get_self_path();
        globals::steamapi_module = kb::loader::load_original_library(self_path, STEAMAPI_DLL);
    }

    void init_hook_mode() {
        LOG_INFO("Detected hook mode");

        kb::hook::init(true);

        kb::dll_monitor::init_listener(STEAMCLIENT_DLL, [](const HMODULE& library) {
            globals::steamclient_module = library;

            DETOUR_STEAMCLIENT(CreateInterface)

            kb::dll_monitor::shutdown_listener();
        });
    }
}
namespace smoke_api {
    void init(const HMODULE module_handle) {
        // FIXME: IMPORTANT! Non ascii paths in directories will result in init errors
        try {
            kb::globals::init_globals(module_handle, PROJECT_NAME);

            config::instance = kb::config::parse<config::Config>();

            if (config::instance.logging) {
                kb::logger::init_file_logger(kb::paths::get_log_path());
            }

            // This kind of timestamp is reliable only for CI builds, as it will reflect the
            // compilation time stamp only when this file gets recompiled.
            LOG_INFO("{} v{} | Compiled at '{}'", PROJECT_NAME, PROJECT_VERSION, __TIMESTAMP__);

            const fs::path exe_path = kb::win_util::get_module_file_name_or_throw(nullptr);
            const auto exe_name = exe_path.filename().string();

            LOG_DEBUG("Process name: '{}' [{}-bit]", exe_name, kb::util::BITNESS);

            override_app_id();

            if (kb::hook::is_hook_mode(module_handle, STEAMAPI_DLL)) {
                init_hook_mode();
            } else {
                init_proxy_mode();
            }

            LOG_INFO("Initialization complete");
        } catch (const std::exception& ex) {
            kb::util::panic(fmt::format("Initialization error: {}", ex.what()));
        }
    }

    void shutdown() {
        try {
            if (globals::steamapi_module != nullptr) {
                kb::win_util::free_library(globals::steamapi_module);
                globals::steamapi_module = nullptr;
            }

            LOG_INFO("Shutdown complete");
        } catch (const std::exception& e) {
            const auto msg = std::format("Shutdown error: {}", e.what());
            LOG_ERROR(msg);
        }

        kb::logger::shutdown();
    }
}