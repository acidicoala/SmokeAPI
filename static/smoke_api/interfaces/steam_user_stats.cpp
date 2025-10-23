#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

#include <koalabox/io.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/paths.hpp>

#include "smoke_api/config.hpp"
#include "smoke_api/interfaces/steam_user_stats.hpp"

namespace smoke_api::steam_user_stats {
    namespace kb = koalabox;

    std::mutex notification_mutex;

    std::string get_iso_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        
        std::tm tm_now{};
#ifdef _WIN32
        gmtime_s(&tm_now, &time_t_now);
#else
        gmtime_r(&time_t_now, &tm_now);
#endif
        
        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

    void write_achievement_notification(
        AppId_t app_id,
        const std::string& event_type,
        const std::string& achievement_id,
        bool unlocked,
        uint32_t current_progress = 0,
        uint32_t max_progress = 0
    ) {
        if (!config::instance.track_achievements) {
            return;
        }

        try {
            std::lock_guard<std::mutex> lock(notification_mutex);

            const auto notification_path = kb::paths::get_self_dir() / "achievements_notification.json";

            // Read existing JSON array or create new one
            nlohmann::json achievements_array = nlohmann::json::array();
            
            if (std::filesystem::exists(notification_path)) {
                try {
                    const auto existing_content = kb::io::read_file(notification_path);
                    if (!existing_content.empty()) {
                        achievements_array = nlohmann::json::parse(existing_content);
                        
                        // Ensure it's an array
                        if (!achievements_array.is_array()) {
                            achievements_array = nlohmann::json::array();
                        }
                    }
                } catch (const std::exception& e) {
                    LOG_WARN("Failed to parse existing achievements file, creating new: {}", e.what());
                    achievements_array = nlohmann::json::array();
                }
            }

            // Create new achievement entry
            nlohmann::ordered_json new_entry;
            new_entry["event"] = event_type;
            new_entry["timestamp"] = get_iso_timestamp();
            
            nlohmann::ordered_json achievement;
            achievement["id"] = achievement_id;
            achievement["unlocked"] = unlocked;
            
            new_entry["achievement"] = achievement;

            // Check if achievement already exists and update it, otherwise append
            bool found = false;
            for (auto& entry : achievements_array) {
                if (entry.contains("achievement") && 
                    entry["achievement"].contains("id") && 
                    entry["achievement"]["id"] == achievement_id) {
                    // Update existing achievement with new data
                    entry["event"] = event_type;
                    entry["timestamp"] = get_iso_timestamp();
                    entry["achievement"]["unlocked"] = unlocked;
                    found = true;
                    LOG_DEBUG("Achievement updated: {}", achievement_id);
                    break;
                }
            }

            if (!found) {
                // Add new achievement
                achievements_array.push_back(new_entry);
                LOG_DEBUG("Achievement added: {}", achievement_id);
            }

            // Write the entire array back to file with pretty printing
            const auto json_str = achievements_array.dump(4);
            
            if (kb::io::write_file(notification_path, json_str)) {
                LOG_DEBUG("Achievement notification written: {}", achievement_id);
            } else {
                LOG_ERROR("Failed to write achievement notification for: {}", achievement_id);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error writing achievement notification: {}", e.what());
        }
    }

    bool GetAchievement(
        const char* function_name,
        const AppId_t app_id,
        const char* achievement_name,
        bool* achieved,
        const std::function<bool()>& original_function
    ) noexcept {
        const auto result = original_function();

        if (config::instance.track_achievements && result && achieved && *achieved) {
            LOG_INFO("{} -> Achievement queried: '{}' (unlocked: {})", function_name, achievement_name, *achieved);
            write_achievement_notification(
                app_id,
                "achievement_queried",
                achievement_name,
                *achieved
            );
        }

        return result;
    }

    bool SetAchievement(
        const char* function_name,
        const AppId_t app_id,
        const char* achievement_name,
        const std::function<bool()>& original_function
    ) noexcept {
        const auto result = original_function();

        if (config::instance.track_achievements && result) {
            LOG_INFO("{} -> Achievement unlocked: '{}'", function_name, achievement_name);
            write_achievement_notification(
                app_id,
                "achievement_unlocked",
                achievement_name,
                true,
                100,
                100
            );
        }

        return result;
    }

    bool ClearAchievement(
        const char* function_name,
        const AppId_t app_id,
        const char* achievement_name,
        const std::function<bool()>& original_function
    ) noexcept {
        const auto result = original_function();

        if (config::instance.track_achievements && result) {
            LOG_INFO("{} -> Achievement cleared: '{}'", function_name, achievement_name);
            write_achievement_notification(
                app_id,
                "achievement_cleared",
                achievement_name,
                false
            );
        }

        return result;
    }

    bool IndicateAchievementProgress(
        const char* function_name,
        const AppId_t app_id,
        const char* achievement_name,
        const uint32_t current_progress,
        const uint32_t max_progress,
        const std::function<bool()>& original_function
    ) noexcept {
        const auto result = original_function();

        if (config::instance.track_achievements && result) {
            LOG_INFO(
                "{} -> Achievement progress: '{}' ({}/{})",
                function_name,
                achievement_name,
                current_progress,
                max_progress
            );
            write_achievement_notification(
                app_id,
                "achievement_progress",
                achievement_name,
                current_progress >= max_progress,
                current_progress,
                max_progress
            );
        }

        return result;
    }

    bool StoreStats(
        const char* function_name,
        const AppId_t app_id,
        const std::function<bool()>& original_function
    ) noexcept {
        const auto result = original_function();

        if (config::instance.track_achievements && result) {
            LOG_DEBUG("{} -> Stats stored to Steam", function_name);
        }

        return result;
    }
}
