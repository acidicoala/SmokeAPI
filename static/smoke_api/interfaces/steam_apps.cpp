#include <set>

#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_apps.hpp"
#include "smoke_api/api.hpp"
#include "smoke_api/cache.hpp"
#include "smoke_api/config.hpp"
#include "smoke_api/types.hpp"

namespace {
    /// Steamworks may max GetDLCCount value at 64, depending on how much unowned DLCs the user has.
    /// Despite this limit, some games with more than 64 DLCs still keep using this method.
    /// This means we have to get extra DLC IDs from local config, remote config, or cache.
    constexpr auto MAX_DLC = 64;

    std::map<uint32_t, std::vector<DLC>> app_dlcs; // NOLINT(cert-err58-cpp)
    std::set<uint32_t> fully_fetched; // NOLINT(cert-err58-cpp)

    std::string get_app_id_log(const uint32_t app_id) {
        return app_id ? fmt::format("App ID: {:>8}, ", app_id) : "";
    }

    /**
     * @param app_id
     * @return boolean indicating if the function was able to successfully fetch DLC IDs from all sources.
     */
    void fetch_and_cache_dlcs(AppId_t app_id) {
        static std::mutex section;
        const std::lock_guard lock(section);

        if(app_id == 0) {
            LOG_ERROR("'{}' -> App ID is 0", __func__);
            app_dlcs[app_id] = {}; // Dummy value to avoid checking for presence on each access
            return;
        }

        // We want to fetch data only once. However, if any of the remote sources have failed
        // previously, we want to attempt fetching again.
        if(fully_fetched.contains(app_id)) {
            return;
        }

        // Any of the sources might fail, so we try to get optimal result
        // by aggregating results from all the sources into a single set.
        std::vector<DLC> aggregated_dlcs;

        const auto append_dlcs = [&](
            const std::vector<DLC>& dlc_list,
            const std::string& source_name
        ) {
            LOG_DEBUG("App ID {} has {} DLCs defined in {}", app_id, dlc_list.size(), source_name);
            // Append DLCs to aggregated DLCs
            std::ranges::copy(dlc_list, std::back_inserter(aggregated_dlcs));
        };

        append_dlcs(smoke_api::config::get_extra_dlcs(app_id), "local config");

        const auto github_dlcs_opt = smoke_api::api::fetch_dlcs_from_github(app_id);
        if(github_dlcs_opt) {
            append_dlcs(*github_dlcs_opt, "GitHub repository");
        }

        const auto steam_dlcs_opt = smoke_api::api::fetch_dlcs_from_steam(app_id);
        if(steam_dlcs_opt) {
            append_dlcs(*steam_dlcs_opt, "Steam API");
        }

        if(github_dlcs_opt && steam_dlcs_opt) {
            fully_fetched.insert(app_id);
        } else {
            append_dlcs(smoke_api::cache::get_dlcs(app_id), "disk cache");
        }

        // Cache DLCs in memory and cache for future use
        app_dlcs[app_id] = aggregated_dlcs;

        smoke_api::cache::save_dlcs(app_id, aggregated_dlcs);
    }
}

namespace smoke_api::steam_apps {
    bool IsDlcUnlocked(
        const std::string& function_name,
        const AppId_t app_id,
        const AppId_t dlc_id,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto unlocked = config::is_dlc_unlocked(
                app_id,
                dlc_id,
                original_function
            );

            LOG_INFO(
                "'{}' -> {}DLC ID: {:>8}, Unlocked: {}",
                function_name,
                get_app_id_log(app_id),
                dlc_id,
                unlocked
            );

            return unlocked;
        } catch(const std::exception& e) {
            LOG_ERROR("'{}' -> Uncaught exception: {}", function_name, e.what());
            return false;
        }
    }

    int GetDLCCount(
        const std::string& function_name,
        const AppId_t app_id,
        const std::function<int()>& original_function
    ) noexcept {
        try {
            const auto total_count = [&](int count) {
                LOG_INFO("'{}' -> Responding with DLC count: {}", function_name, count);
                return count;
            };

            if(app_id != 0) {
                LOG_DEBUG("'{}' -> App ID: {}", function_name, app_id);
            }

            const auto original_count = original_function();
            LOG_DEBUG("'{}' -> Original DLC count: {}", function_name, original_count);

            if(original_count < MAX_DLC) {
                return total_count(original_count);
            }

            LOG_DEBUG(
                "Game has {} or more DLCs. Fetching DLCs from remote sources.",
                original_count
            );

            fetch_and_cache_dlcs(app_id);

            return total_count(static_cast<int>(app_dlcs[app_id].size()));
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Uncaught exception: {}", function_name, e.what());
            return 0;
        }
    }

    bool GetDLCDataByIndex(
        const std::string& function_name,
        const AppId_t app_id,
        int iDLC,
        AppId_t* pDlcId,
        bool* pbAvailable,
        char* pchName,
        const int cchNameBufferSize,
        const std::function<bool()>& original_function,
        const std::function<bool()>& is_originally_unlocked
    ) noexcept {
        try {
            LOG_DEBUG("'{}' -> {}index: {:>3}", function_name, get_app_id_log(app_id), iDLC);

            const auto print_dlc_info = [&](const std::string& tag) {
                LOG_INFO(
                    R"('{}' -> [{:^12}] {}index: {:>3}, DLC ID: {:>8}, available: {:5}, name: "{}")",
                    function_name,
                    tag,
                    get_app_id_log(app_id),
                    iDLC,
                    *pDlcId,
                    *pbAvailable,
                    pchName
                );
            };

            const auto output_dlc = [&](const DLC& dlc) {
                // Fill the output pointers
                *pDlcId = dlc.get_id();
                *pbAvailable = config::is_dlc_unlocked(app_id, *pDlcId, is_originally_unlocked);

                auto name = dlc.get_name();
                name = name.substr(0, cchNameBufferSize + 1);
                memcpy_s(pchName, cchNameBufferSize, name.c_str(), name.size());
            };

            if(app_dlcs.contains(app_id)) {
                const auto& dlcs = app_dlcs[app_id];

                if(iDLC >= 0 && iDLC < dlcs.size()) {
                    output_dlc(dlcs[iDLC]);
                    print_dlc_info("injected");
                    return true;
                }

                LOG_WARN("'{}' -> Out of bounds DLC index: {}", function_name, iDLC);
                return false;
            }

            const auto success = original_function();

            if(success) {
                *pbAvailable = config::is_dlc_unlocked(
                    app_id,
                    *pDlcId,
                    [&] {
                        return *pbAvailable;
                    }
                );
                print_dlc_info("original");
            } else {
                LOG_WARN("'{}' -> original call failed for index: {}", function_name, iDLC);
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("'{}' -> Uncaught exception: {}", function_name, e.what());
            return false;
        }
    }
}
