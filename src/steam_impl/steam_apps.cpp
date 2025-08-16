#include <steam_impl/steam_apps.hpp>
#include <common/app_cache.hpp>
#include <smoke_api/config.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>
#include <core/types.hpp>
#include <core/api.hpp>

namespace steam_apps {
    /// Steamworks may max GetDLCCount value at 64, depending on how much unowned DLCs the user has.
    /// Despite this limit, some games with more than 64 DLCs still keep using this method.
    /// This means we have to get extra DLC IDs from local config, remote config, or cache.
    constexpr auto MAX_DLC = 64;

    Map<AppId_t, Vector<DLC>> app_dlcs; // NOLINT(cert-err58-cpp)
    Set<AppId_t> fully_fetched; // NOLINT(cert-err58-cpp)

    String get_app_id_log(const AppId_t app_id) {
        return app_id ? fmt::format("App ID: {:>8}, ", app_id) : "";
    }

    /**
     * @param app_id
     * @return boolean indicating if the function was able to successfully fetch DLC IDs from all sources.
     */
    void fetch_and_cache_dlcs(AppId_t app_id) {
        static Mutex mutex;
        const MutexLockGuard guard(mutex);

        if (app_id == 0) {
            LOG_ERROR("{} -> App ID is 0", __func__);
            app_dlcs[app_id] = {}; // Dummy value to avoid checking for presence on each access
            return;
        }

        // We want to fetch data only once. However, if any of the remote sources have failed
        // previously, we want to attempt fetching again.
        if (fully_fetched.contains(app_id)) {
            return;
        }

        // Any of the sources might fail, so we try to get optimal result
        // by aggregating results from all the sources into a single set.
        Vector<DLC> aggregated_dlcs;

        const auto append_dlcs = [&](const Vector<DLC>& source, const String& source_name) {
            LOG_DEBUG("App ID {} has {} DLCs defined in {}", app_id, source.size(), source_name);
            aggregated_dlcs < append > source;
        };

        append_dlcs(smoke_api::config::get_extra_dlcs(app_id), "local config");

        const auto github_dlcs_opt = api::fetch_dlcs_from_github(app_id);
        if (github_dlcs_opt) {
            append_dlcs(*github_dlcs_opt, "GitHub repository");
        }

        const auto steam_dlcs_opt = api::fetch_dlcs_from_steam(app_id);
        if (steam_dlcs_opt) {
            append_dlcs(*steam_dlcs_opt, "Steam API");
        }

        if (github_dlcs_opt && steam_dlcs_opt) {
            fully_fetched.insert(app_id);
        } else {
            append_dlcs(smoke_api::app_cache::get_dlcs(app_id), "disk cache");
        }

        // Cache DLCs in memory and cache for future use
        app_dlcs[app_id] = aggregated_dlcs;

        smoke_api::app_cache::save_dlcs(app_id, aggregated_dlcs);
    }

    bool IsDlcUnlocked(
        const String& function_name,
        AppId_t app_id,
        AppId_t dlc_id,
        const Function<bool()>& original_function
    ) {
        try {
            const auto unlocked = smoke_api::config::is_dlc_unlocked(app_id, dlc_id, original_function);

            LOG_INFO("{} -> {}DLC ID: {:>8}, Unlocked: {}", function_name, get_app_id_log(app_id), dlc_id, unlocked);

            return unlocked;
        } catch (const Exception& e) {
            LOG_ERROR("Uncaught exception: {}", e.what());
            return false;
        }
    }

    int GetDLCCount(const String& function_name, const AppId_t app_id, const Function<int()>& original_function) {
        try {
            const auto total_count = [&](int count) {
                LOG_INFO("{} -> Responding with DLC count: {}", function_name, count);
                return count;
            };

            if (app_id != 0) {
                LOG_DEBUG("{} -> App ID: {}", function_name, app_id);
            }

            const auto original_count = original_function();
            LOG_DEBUG("{} -> Original DLC count: {}", function_name, original_count);

            if (original_count < MAX_DLC) {
                return total_count(original_count);
            }

            LOG_DEBUG("Game has {} or more DLCs. Fetching DLCs from remote sources.", original_count);

            fetch_and_cache_dlcs(app_id);

            return total_count(static_cast<int>(app_dlcs[app_id].size()));
        } catch (const Exception& e) {
            LOG_ERROR(" Uncaught exception: {}", function_name, e.what());
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
        const Function<bool()>& original_function,
        const Function<bool(AppId_t)>& is_originally_unlocked
    ) {
        try {
            LOG_DEBUG("{} -> {}index: {:>3}", function_name, get_app_id_log(app_id), iDLC);

            const auto print_dlc_info = [&](const String& tag) {
                LOG_INFO(
                    R"({} -> [{:^12}] {}index: {:>3}, DLC ID: {:>8}, available: {:5}, name: "{}")",
                    function_name, tag, get_app_id_log(app_id), iDLC, *pDlcId, *pbAvailable, pchName
                );
            };

            const auto inject_dlc = [&](const DLC& dlc) {
                // Fill the output pointers
                *pDlcId = dlc.get_id();
                *pbAvailable = smoke_api::config::is_dlc_unlocked(
                    app_id, *pDlcId, [&]() {
                        return is_originally_unlocked(*pDlcId);
                    }
                );

                auto name = dlc.get_name();
                name = name.substr(0, cchNameBufferSize + 1);
                memcpy_s(pchName, cchNameBufferSize, name.c_str(), name.size());
            };

            if (app_dlcs.contains(app_id)) {
                const auto& dlcs = app_dlcs[app_id];

                if (iDLC >= 0 && iDLC < dlcs.size()) {
                    inject_dlc(dlcs[iDLC]);
                    print_dlc_info("injected");
                    return true;
                }

                LOG_WARN("{} -> Out of bounds DLC index: {}", function_name, iDLC);
                return false;
            }

            const auto success = original_function();

            if (success) {
                *pbAvailable = smoke_api::config::is_dlc_unlocked(
                    app_id, *pDlcId, [&]() { return *pbAvailable; }
                );
                print_dlc_info("original");
            } else {
                LOG_WARN("{} -> original call failed for index: {}", function_name, iDLC);
            }

            return success;
        } catch (const Exception& e) {
            LOG_ERROR("{} -> Uncaught exception: {}", function_name, e.what());
            return false;
        }
    }

}
