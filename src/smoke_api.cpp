#include <koalabox/config.hpp>
#include <koalabox/dll_monitor.hpp>
#include <koalabox/globals.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/path.hpp>
#include <koalabox/paths.hpp>
#include <koalabox/str.hpp>
#include <koalabox/util.hpp>
#include <koalabox/win.hpp>

#include "build_config.h"

#include "smoke_api.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/steamclient/steamclient.hpp"
#include "steam_api/exports/steam_api.hpp"

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

    void init_proxy_mode() {
        LOG_INFO("Detected proxy mode");

        const auto self_path = kb::paths::get_self_dir();
        smoke_api::steamapi_module = kb::loader::load_original_library(self_path, STEAMAPI_DLL);
    }

    void init_hook_mode() {
        LOG_INFO("Detected hook mode");

        kb::dll_monitor::init_listener(
            {STEAMCLIENT_DLL, STEAMAPI_DLL},
            [&](const HMODULE& module_handle, const std::string& library_name) {
                if(kb::str::eq(library_name, STEAMCLIENT_DLL)) {
                    KB_HOOK_DETOUR_MODULE(CreateInterface, module_handle);
                } else if(kb::str::eq(library_name, STEAMAPI_DLL)) {
                    KB_HOOK_DETOUR_MODULE(SteamAPI_RestartAppIfNecessary, module_handle);
                    KB_HOOK_DETOUR_MODULE(SteamAPI_Shutdown, module_handle);
                }
            }
        );
    }
}

namespace smoke_api {
    HMODULE steamapi_module = nullptr;
    bool hook_mode = false;

    void init(const HMODULE module_handle) {
        try {
            kb::globals::init_globals(module_handle, PROJECT_NAME);

            config::instance = kb::config::parse<config::Config>();

            if(config::instance.logging) {
                kb::logger::init_file_logger(kb::paths::get_log_path());
            }

            // This kind of timestamp is reliable only for CI builds, as it will reflect the
            // compilation time stamp only when this file gets recompiled.
            LOG_INFO("{} v{} | Compiled at '{}'", PROJECT_NAME, PROJECT_VERSION, __TIMESTAMP__);

            const auto exe_path = kb::win::get_module_path(nullptr);
            const auto exe_name = kb::path::to_str(exe_path.filename());

            LOG_DEBUG("Process name: '{}' [{}-bit]", exe_name, kb::util::BITNESS);

            // We need to hook functions in either mode
            kb::hook::init(true);

            if(kb::hook::is_hook_mode(module_handle, STEAMAPI_DLL)) {
                hook_mode = true;
                init_hook_mode();
            } else {
                init_proxy_mode();
            }

            LOG_INFO("Initialization complete");
        } catch(const std::exception& ex) {
            kb::util::panic(fmt::format("Initialization error: {}", ex.what()));
        }
    }

    void shutdown() {
        try {
            if(steamapi_module != nullptr) {
                kb::win::free_library(steamapi_module);
                steamapi_module = nullptr;
            }

            LOG_INFO("Shutdown complete");
        } catch(const std::exception& e) {
            const auto msg = std::format("Shutdown error: {}", e.what());
            LOG_ERROR(msg);
        }

        kb::logger::shutdown();
    }
}
