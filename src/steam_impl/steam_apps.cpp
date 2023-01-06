#include <steam_impl/steam_apps.hpp>
#include <cpr/cpr.h>
#include <koalabox/io.hpp>
#include <koalabox/http_client.hpp>
#include <core/cache.hpp>
#include <core/config.hpp>
#include <smoke_api/smoke_api.hpp>

namespace steam_apps {
    using namespace smoke_api;

    /// Steamworks may max GetDLCCount value at 64, depending on how much unowned DLCs the user has.
    /// Despite this limit, some games with more than 64 DLCs still keep using this method.
    /// This means we have to get extra DLC IDs from local config, remote config, or cache.
    constexpr auto MAX_DLC = 64;

    // Key: App ID, Value: DLC ID
    Map<AppId_t, int> original_dlc_count_map; // NOLINT(cert-err58-cpp)
    Vector<AppId_t> cached_dlcs;

    /**
     * @param app_id
     * @return boolean indicating if the function was able to successfully fetch DLC IDs from all sources.
     */
    bool fetch_and_cache_dlcs(AppId_t app_id) {
        if (not app_id) {
            try {
                app_id = steam_functions::get_app_id_or_throw();
                // TODO: Check what it returns in koalageddon mode
                logger->info("Detected App ID: {}", app_id);
            } catch (const Exception& ex) {
                logger->error("Failed to get app ID: {}", ex.what());
                return false;
            }
        }

        auto total_success = true;
        const auto app_id_str = std::to_string(app_id);

        const auto fetch_from_steam = [&]() {
            Vector<AppId_t> dlcs;

            try {
                // TODO: Refactor into api namespace
                const auto url = fmt::format("https://store.steampowered.com/dlc/{}/ajaxgetdlclist", app_id_str);
                const auto json = http_client::fetch_json(url);

                if (json["success"] != 1) {
                    throw util::exception("Web API responded with 'success' != 1");
                }

                for (const auto& dlc: json["dlcs"]) {
                    const auto app_id = dlc["appid"].get<String>();
                    dlcs.emplace_back(std::stoi(app_id));
                }
            } catch (const Exception& e) {
                logger->error("Failed to fetch dlc list from steam api: {}", e.what());
                total_success = false;
            }

            return dlcs;
        };

        const auto fetch_from_github = [&]() {
            Vector<AppId_t> dlcs;

            try {
                const String url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/steam/v1/dlc.json";
                const auto json = http_client::fetch_json(url);

                if (json.contains(app_id_str)) {
                    dlcs = json[app_id_str].get<decltype(dlcs)>();
                }
            } catch (const Exception& e) {
                logger->error("Failed to fetch extra dlc list from github api: {}", e.what());
                total_success = false;
            }

            return dlcs;
        };

        const auto steam_dlcs = fetch_from_steam();
        const auto github_dlcs = fetch_from_github();

        // Any of the sources might fail, so we try to get optimal result
        // by combining results from all the sources into a single set.
        Set<AppId_t> combined_dlcs;
        combined_dlcs.insert(steam_dlcs.begin(), steam_dlcs.end());
        combined_dlcs.insert(github_dlcs.begin(), github_dlcs.end());
        // There is no need to insert cached entries if both steam and GitHub requests were successful.
        if (!total_success) {
            const auto cache_dlcs = cache::get_dlc_ids(app_id);
            combined_dlcs.insert(cached_dlcs.begin(), cached_dlcs.end());
        }

        // We then transfer that set into a list because we need DLCs to be accessible via index.
        cached_dlcs.clear();
        cached_dlcs.insert(cached_dlcs.begin(), combined_dlcs.begin(), combined_dlcs.end());

        cache::save_dlc_ids(app_id, cached_dlcs);

        return total_success;
    }

    String get_app_id_log(const AppId_t app_id) {
        return app_id ? fmt::format("App ID: {}, ", app_id) : "";
    }

    bool IsDlcUnlocked(
        const String& function_name,
        AppId_t app_id, AppId_t dlc_id,
        const std::function<bool()>& original_function
    ) {
        try {
            const auto unlocked = config::is_dlc_unlocked(app_id, dlc_id, original_function);

            logger->info("{} -> {}DLC ID: {}, Unlocked: {}", function_name, get_app_id_log(app_id), dlc_id, unlocked);

            return unlocked;
        } catch (const Exception& e) {
            logger->error("{} -> Uncaught exception: {}", function_name, e.what());
            return false;
        }
    }

    int GetDLCCount(const String& function_name, const AppId_t app_id, const std::function<int()>& original_function) {
        try {
            const auto total_count = [&](int count) {
                logger->info("{} -> Responding with DLC count: {}", function_name, count);
                return count;
            };

            if (app_id != 0) {
                logger->debug("{} -> App ID: {}", function_name, app_id);
            }

            const auto original_count = original_function();
            original_dlc_count_map[app_id] = original_count;
            logger->debug("{} -> Original DLC count: {}", function_name, original_count);

            if (original_count < MAX_DLC) {
                return total_count(original_count);
            }

            // We need to fetch DLC IDs from all possible sources at this point

            const auto injected_count = static_cast<int>(config::instance.extra_dlc_ids.size());
            logger->debug("{} -> Injected DLC count: {}", function_name, injected_count);

            // Maintain a list of app_ids for which we have already fetched and cached DLC IDs
            static Set<AppId_t> cached_apps;
            if (!cached_apps.contains(app_id)) {
                static std::mutex mutex;
                const std::lock_guard<std::mutex> guard(mutex);

                logger->debug("Game has {} or more DLCs. Fetching DLCs from remote sources.", MAX_DLC);

                if (fetch_and_cache_dlcs(app_id)) {
                    cached_apps.insert(app_id);
                }
            }

            const auto cached_count = static_cast<int>(cached_dlcs.size());
            logger->debug("{} -> Cached DLC count: {}", function_name, cached_count);

            return total_count(injected_count + cached_count);
        } catch (const Exception& e) {
            logger->error("{} -> Uncaught exception: {}", function_name, e.what());
            return 0;
        }
    }

    bool GetDLCDataByIndex(
        const String& function_name,
        AppId_t app_id,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        int cchNameBufferSize,
        const std::function<bool()>& original_function
    ) {
        try {
            const auto print_dlc_info = [&](const String& tag) {
                logger->info(
                    "{} -> [{}] {}index: {}, DLC ID: {}, available: {}, name: '{}'",
                    function_name, tag, get_app_id_log(app_id), iDLC, *pDlcId, *pbAvailable, pchName
                );
            };

            const auto inject_dlc = [&](const String& tag, const Vector<AppId_t>& dlc_ids, const int index) {
                const auto dlc_id = dlc_ids[index];

                // Fill the output pointers
                *pDlcId = dlc_id;
                *pbAvailable = config::is_dlc_unlocked(app_id, dlc_id, []() { return true; });

                auto name = fmt::format("DLC #{} with ID: {} ", iDLC, dlc_id);
                name = name.substr(0, cchNameBufferSize);
                *name.rbegin() = '\0';
                memcpy_s(pchName, cchNameBufferSize, name.c_str(), name.size());

                print_dlc_info(tag);
                return true;
            };

            const auto get_original_dlc_count = [](const AppId_t& app_id) {
                if (original_dlc_count_map.contains(app_id)) {
                    return original_dlc_count_map[app_id];
                }

                return 0;
            };

            const auto original_count = get_original_dlc_count(app_id);

            // Original count less than MAX_DLC implies that we need to redirect the call to original function.

            if (original_count < MAX_DLC) {
                const auto success = original_function();

                if (success) {
                    *pbAvailable = config::is_dlc_unlocked(app_id, *pDlcId, [&]() { return *pbAvailable; });
                    print_dlc_info("original");
                } else {
                    logger->warn("{} -> original call failed for index: {}", function_name, iDLC);
                }
                return success;
            }

            // We must have had cached DLC IDs at this point.
            // It does not matter if we begin the list with injected DLC IDs or cached ones.
            // However, we must be consistent at all times. Hence, the convention will be that
            // injected DLCs will be followed by cached DLCs in the following manner:
            // [injected-dlc-0, injected-dlc-1, ..., cached-dlc-0, cached-dlc-1, ...]

            if (iDLC < 0) {
                logger->warn("{} -> Out of bounds DLC index: {}", function_name, iDLC);
            }

            const int local_dlc_count = static_cast<int>(config::instance.extra_dlc_ids.size());
            if (iDLC < local_dlc_count) {
                return inject_dlc("local config", config::instance.extra_dlc_ids, iDLC);
            }

            const auto adjusted_index = iDLC - local_dlc_count;
            const int cached_dlc_count = static_cast<int>(cached_dlcs.size());
            if (iDLC < cached_dlc_count) {
                return inject_dlc("memory cache", cached_dlcs, adjusted_index);
            }

            logger->error(
                "{} -> Out of bounds DLC index: {}, local dlc count: {}, cached dlc count: {}",
                function_name, iDLC, local_dlc_count, cached_dlc_count
            );

            return false;
        } catch (const Exception& e) {
            logger->error("{} -> Uncaught exception: {}", function_name, e.what());
            return false;
        }
    }
}
