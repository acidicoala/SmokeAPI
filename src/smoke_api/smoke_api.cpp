#include <regex>
#include <set>

#include <koalabox/config.hpp>
#include <koalabox/lib_monitor.hpp>
#include <koalabox/globals.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/lib.hpp>
#include <koalabox/path.hpp>
#include <koalabox/paths.hpp>
#include <koalabox/util.hpp>

#include "smoke_api.hpp"

#include "smoke_api/config.hpp"
#include "smoke_api/steamclient/steamclient.hpp"
#include "steam_api/steam_interfaces.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

#include "build_config.h"

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

    void* original_steamapi_handle = nullptr;
    bool is_hook_mode;

    std::set<std::string> find_steamclient_versions(void* steamapi_handle) {
        if(!steamapi_handle) {
            kb::util::panic("Invalid state. steamapi_handle is null.");
        }

        std::set<std::string> versions;

        const auto rdata_section = kb::lib::get_section_or_throw(steamapi_handle, kb::lib::CONST_STR_SECTION);
        const auto rdata = rdata_section.to_string();

        const std::regex pattern(R"(SteamClient\d{3})");
        const auto matches_begin = std::sregex_iterator(rdata.begin(), rdata.end(), pattern);
        const auto matches_end = std::sregex_iterator();

        for(std::sregex_iterator i = matches_begin; i != matches_end; ++i) {
            versions.insert(i->str());
        }

        return versions;
    }

    void warn_if_late_injection(const std::string& steamclient_version) {
#ifdef KB_WIN
        if(kb::util::is_wine_env()) {
            return;
        }

        LOG_WARN(
            "'{}' was already initialized. SmokeAPI might not work as expected.", steamclient_version
        );
        LOG_WARN(
            "Probable cause: SmokeAPI was injected too late. If possible, try injecting it earlier."
        );
        LOG_WARN("NOTE: You can safely ignore this warning if running under Proton or native Linux");
#endif
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    bool on_steamclient_loaded(void* steamclient_handle) {
        KB_HOOK_DETOUR_MODULE(CreateInterface, steamclient_handle);

        if(!is_hook_mode) {
            return true;
        }

        // Check for late hooking

        static const auto CreateInterface$ = KB_LIB_GET_FUNC(steamclient_handle, CreateInterface);

        if(auto* steamapi_handle = kb::lib::get_library_handle(STEAM_API_MODULE)) {
            // SteamAPI might have been initialized.
            // Hence, we need to query SteamClient interfaces and hook them if needed.
            const auto steamclient_versions = find_steamclient_versions(steamapi_handle);
            for(const auto& steamclient_version : steamclient_versions) {
                if(CreateInterface$(steamclient_version.c_str(), nullptr)) {
                    warn_if_late_injection(steamclient_version);

                    steam_interfaces::hook_steamclient_interface(steamclient_handle, steamclient_version);
                } else {
                    LOG_INFO("'{}' has not been initialized. Waiting for initialization.", steamclient_version);
                }
            }
        }

        return true;
    }

    void init_lib_monitor() {
        kb::lib_monitor::init_listener({{STEAMCLIENT_DLL, on_steamclient_loaded}});
    }
}

namespace smoke_api {
    void init(void* module_handle) {
        try {
            kb::globals::init_globals(module_handle, PROJECT_NAME);

            config::instance = kb::config::parse<config::Config>();

            if(config::instance.logging) {
                kb::logger::init_file_logger(kb::paths::get_log_path());
            } else {
                kb::logger::init_null_logger();
            }

            LOG_INFO("{} v{} | Built at '{}'", PROJECT_NAME, PROJECT_VERSION, __TIMESTAMP__);
            LOG_DEBUG("Parsed config:\n{}", nlohmann::ordered_json(config::instance).dump(2));

            const auto exe_path = kb::lib::get_fs_path(nullptr);
            const auto exe_name = kb::path::to_str(exe_path.filename());

            LOG_DEBUG("Process name: '{}' [{}-bit]", exe_name, kb::util::BITNESS);
            LOG_DEBUG("Self name: '{}'", kb::path::to_str(kb::lib::get_fs_path(module_handle).filename()));

            // We need to hook functions in either mode
            kb::hook::init(true);

            if(kb::hook::is_hook_mode(module_handle, STEAM_API_MODULE)) {
                LOG_INFO("Detected hook mode");

                is_hook_mode = true;
                init_lib_monitor();
            } else {
                LOG_INFO("Detected proxy mode");

                is_hook_mode = true;
                init_lib_monitor();

                const auto self_path = kb::paths::get_self_dir();
                original_steamapi_handle = kb::lib::load_original_library(self_path, STEAM_API_MODULE);
            }

            LOG_INFO("Initialization complete");
        } catch(const std::exception& e) {
            kb::util::panic(std::format("Initialization error: {}", e.what()));
        }
    }

    void shutdown() {
        try {
            if(original_steamapi_handle != nullptr) {
                kb::lib::unload_library(original_steamapi_handle);
                original_steamapi_handle = nullptr;
            }

            if(kb::lib_monitor::is_initialized()) {
                kb::lib_monitor::shutdown_listener();
            }

            // TODO: Unhook everything

            LOG_INFO("Shutdown complete");
        } catch(const std::exception& e) {
            LOG_ERROR("Shutdown error: {}", e.what());
        }

        kb::logger::shutdown();
    }

    AppId_t get_app_id() {
        static AppId_t app_id = 0;
        if(app_id) {
            return app_id; // cached value
        }

        try {
            if(const auto app_id_str = kb::util::get_env("SteamAppId")) {
                app_id = std::stoi(*app_id_str);
                LOG_DEBUG("Found AppID from environment: {}", app_id);

                return app_id;
            }
        } catch(std::exception&) {
            LOG_WARN("No SteamAppId in environment. Falling back to ISteamUtils::GetAppID.");
        }

        // TODO: Then try to read steam_appid.txt here. SteamAppId env var is not available when it's present.

        try {
            DECLARE_ARGS();

            THIS = CreateInterface("SteamClient007", nullptr);
            if(!THIS) {
                LOG_ERROR("Failed to create SteamClient interface");
                return 0;
            }

            THIS = ISteamClient_GetISteamGenericInterface(ARGS(1, 1, "SteamUtils002"));
            if(!THIS) {
                LOG_ERROR("Failed to get SteamUtils interface");
                return 0;
            }

            app_id = ISteamUtils_GetAppID(ARGS());
            if(!app_id) {
                LOG_ERROR("ISteamUtils::GetAppID returned 0");
                return 0;
            }

            LOG_DEBUG("Found AppID from ISteamUtils: {}", app_id);
            return app_id;
        } catch(const std::exception& e) {
            LOG_ERROR("Failed to get app id: {}", e.what());
            return 0;
        }
    }
}
