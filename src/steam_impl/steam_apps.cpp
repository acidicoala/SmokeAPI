#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

#include <koalabox/io.hpp>

#include <cpr/cpr.h>

using namespace smoke_api;

constexpr auto max_dlc = 64;

Vector<AppId_t> cached_dlcs;
int original_dlc_count = 0;

Path get_cache_path() {
    static const auto path = self_directory / "SmokeAPI.cache.json";
    return path;
}

void read_from_cache(const String& app_id_str) {
    try {
        const auto text = io::read_file(get_cache_path());
        if (text.empty()) {
            return;
        }

        auto json = nlohmann::json::parse(text);

        cached_dlcs = json[app_id_str]["dlc"].get<decltype(cached_dlcs)>();

        logger->debug("Read {} DLCs from cache", cached_dlcs.size());
    } catch (const Exception& ex) {
        logger->error("Error reading DLCs from cache: {}", ex.what());
    }

}

void save_to_cache(const String& app_id_str) {
    try {
        logger->debug("Saving {} DLCs to cache", cached_dlcs.size());

        const nlohmann::json json = {
            {app_id_str, {
                {"dlc", cached_dlcs}
            }}
        };

        io::write_file(get_cache_path(), json.dump(2));
    } catch (const Exception& ex) {
        logger->error("Error saving DLCs to cache: {}", ex.what());
    }
}

void fetch_and_cache_dlcs(AppId_t app_id) {
    if (not app_id) {
        try {
            app_id = steam_functions::get_app_id_or_throw();
            logger->info("Detected App ID: {}", app_id);
        } catch (const Exception& ex) {
            logger->error("Failed to get app ID: {}", ex.what());
            return;
        }
    }

    const auto app_id_str = std::to_string(app_id);

    const auto fetch_from_steam = [&]() {
        const auto url = fmt::format("https://store.steampowered.com/dlc/{}/ajaxgetdlclist", app_id_str);
        const auto res = cpr::Get(cpr::Url{url});

        if (res.status_code != cpr::status::HTTP_OK) {
            throw util::exception(
                "Steam Web API didn't responded with HTTP_OK result. Code: {}, Error: {},\n"
                "Headers:\n{}\nBody:\n{}",
                res.status_code, (int) res.error.code, res.raw_header, res.text
            );
        }

        const auto json = nlohmann::json::parse(res.text);


        if (json["success"] != 1) {
            throw util::exception("Web API responded with 'success': 1.");
        }

        Vector<AppId_t> dlcs;

        for (const auto& dlc: json["dlcs"]) {
            const auto app_id = dlc["appid"].get<String>();
            dlcs.emplace_back(std::stoi(app_id));
        }

        return dlcs;
    };

    const auto fetch_from_github = [&]() {
        const String url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/steam/v1/dlc.json";
        const auto res = cpr::Get(cpr::Url{url});

        if (res.status_code != cpr::status::HTTP_OK) {
            throw util::exception(
                "Github Web API didn't responded with HTTP_OK result. Code: {}, Error: {},\n"
                "Headers:\n{}\nBody:\n{}",
                res.status_code, (int) res.error.code, res.raw_header, res.text
            );
        }

        const auto json = nlohmann::json::parse(res.text);

        if (json.contains(app_id_str)) {
            return json[app_id_str].get<decltype(cached_dlcs)>();
        }

        return Vector<AppId_t>{};
    };

    try {
        read_from_cache(app_id_str);

        auto list1 = fetch_from_steam();
        auto list2 = fetch_from_github();
        list1.insert(list1.end(), list2.begin(), list2.end());
        Set<AppId_t> fetched_dlcs(list1.begin(), list1.end());

        if (fetched_dlcs.size() > cached_dlcs.size()) {
            cached_dlcs = Vector<AppId_t>(fetched_dlcs.begin(), fetched_dlcs.end());
        }

        save_to_cache(app_id_str);
    } catch (const Exception& ex) {
        logger->error("Failed to fetch DLC: {}", ex.what());
    }
}

String get_app_id_log(const AppId_t app_id) {
    return app_id ? fmt::format("App ID: {}, ", app_id) : "";
}

namespace steam_apps {

    bool IsDlcUnlocked(const String& function_name, AppId_t app_id, AppId_t dlc_id) {
        const auto app_id_unlocked = not app_id or should_unlock(app_id); // true if app_id == 0
        const auto dlc_id_unlocked = should_unlock(dlc_id);

        const auto installed = app_id_unlocked and dlc_id_unlocked;

        logger->info("{} -> {}DLC ID: {}, Unlocked: {}", function_name, get_app_id_log(app_id), dlc_id, installed);

        return installed;
    }

//    std::mutex section;
    int GetDLCCount(const String& function_name, const AppId_t app_id, const std::function<int()>& original_function) {
        try {
//            std::lock_guard<std::mutex> guard(section);

            const auto total_count = [&](int count) {
                logger->info("{} -> Responding with DLC count: {}", function_name, count);
                return count;
            };

            if (app_id) {
                logger->debug("{} -> App ID: {}", function_name, app_id);
            }

            // Compute count only once // FIXME: This doesn't work in Koalageddon mode
            original_dlc_count = original_function();
            logger->debug("{} -> Original DLC count: {}", function_name, original_dlc_count);

            const auto injected_count = static_cast<int>(config.dlc_ids.size());
            logger->debug("{} -> Injected DLC count: {}", function_name, injected_count);


            if (original_dlc_count < max_dlc) {
                return total_count(original_dlc_count + injected_count);
            }

            // Steamworks may max out this value at 64, depending on how much unowned DLCs the user has.
            // Despite this limit, some games with more than 64 DLCs still keep using this method.
            // This means we have to fetch full list of IDs from web api.
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                logger->debug("Game has {} or more DLCs. Fetching DLCs from a web API.", max_dlc);
                fetch_and_cache_dlcs(app_id);
            });

            const auto fetched_count = static_cast<int>(cached_dlcs.size());
            logger->debug("{} -> Fetched/cached DLC count: {}", function_name, fetched_count);

            return total_count(fetched_count + injected_count);
        } catch (const Exception& ex) {
            logger->error("{} -> {}", function_name, ex.what());
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
        const auto print_dlc_info = [&](const String& tag) {
            logger->info(
                "{} -> [{}] {}index: {}, DLC ID: {}, available: {}, name: '{}'",
                function_name, tag, get_app_id_log(app_id), iDLC, *pDlcId, *pbAvailable, pchName
            );
        };

        const auto fill_dlc_info = [&](const AppId_t id) {
            *pDlcId = id;
            *pbAvailable = should_unlock(id);

            auto name = fmt::format("DLC #{} with ID: {} ", iDLC, id);
            name = name.substr(0, cchNameBufferSize);
            *name.rbegin() = '\0';
            memcpy_s(pchName, cchNameBufferSize, name.c_str(), name.size());
        };

        const auto inject_dlc = [&](const int index) {
            if (index >= config.dlc_ids.size()) {
                logger->error("{} -> Out of bounds injected index: {}", function_name, index);
                return false;
            }

            const auto dlc_id = config.dlc_ids[index];
            fill_dlc_info(dlc_id);
            print_dlc_info("injected");
            return true;
        };

        // Original response
        if (cached_dlcs.empty()) {
            // Original DLC index
            if (iDLC < original_dlc_count) {
                const auto success = original_function();

                if (success) {
                    *pbAvailable = should_unlock(*pDlcId);
                    print_dlc_info("original");
                } else {
                    logger->warn("{} -> original function failed for index: {}", function_name, iDLC);
                }
                return success;
            }

            // Injected DLC index (after original)
            const auto index = iDLC - original_dlc_count;
            return inject_dlc(index);
        }

        // Cached response
        const auto total_size = cached_dlcs.size() + config.dlc_ids.size();
        if (iDLC < 0 or iDLC >= total_size) {
            logger->error(
                "{} -> Game accessed out of bounds DLC index: {}. Total size: {}",
                function_name, iDLC, total_size
            );
            return false;
        }

        // Cached index
        if (iDLC < cached_dlcs.size()) {
            const auto dlc_id = cached_dlcs[iDLC];
            fill_dlc_info(dlc_id);
            print_dlc_info("cached");
            return true;
        }

        // Injected DLC index (after cached)

        const auto index = iDLC - static_cast<int>(cached_dlcs.size());
        return inject_dlc(index);
    }
}
