#include <filesystem>
#include <iostream>
#include <random>
#include <regex>
#include <string>

#include <koalabox/http_client.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/path.hpp>
#include <koalabox/str.hpp>
#include <koalabox/zip.hpp>

namespace {
    namespace fs = std::filesystem;
    namespace kb = koalabox;

    std::string generate_random_string() {
        static constexpr char charset[] = "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        thread_local std::mt19937_64 rng{std::random_device{}()};
        thread_local std::uniform_int_distribution<std::size_t> dist(0, sizeof(charset) - 2);

        constexpr auto length = 16;
        std::string result;
        result.reserve(length);
        for(std::size_t i = 0; i < length; ++i) {
            result += charset[dist(rng)];
        }

        return result;
    }

    void print_help() {
        std::cout << "Steamworks SDK downloader for SmokeAPI v1.0" << std::endl
            << "Usage:   steamworks_downloader version1 version2 ... versionN" << std::endl
            << "Example: steamworks_downloader 100 158a 162" << std::endl
            << "Alternative usage: steamworks_downloader C:/path/to/downloaded_sdk/"
            << "SDK version list available at: "
            << "https://partner.steamgames.com/downloads/list" << std::endl;
    }

    void unzip_sdk(const fs::path& zip_file_path, const fs::path& unzip_dir) {
        kb::zip::extract_files(
            zip_file_path,
            [&](const std::string& name, const bool) {
                if(name.starts_with("sdk/public/steam/") && name.ends_with(".h")) {
                    return unzip_dir / "headers/steam" / fs::path(name).filename();
                }

                // Windows binaries
                if(
                    name.starts_with("sdk/redistributable_bin/") &&
                    name.ends_with(".dll") &&
                    name.contains("steam_api")
                ) {
                    return unzip_dir / "binaries" / fs::path(name).filename();
                }

                // Linux binaries
                if(
                    name.starts_with("sdk/redistributable_bin/linux") &&
                    name.ends_with("libsteam_api.so")
                    ) {
                    return unzip_dir / "binaries" / name.substr(name.find("linux"));
                }

                return fs::path();
            }
        );
    }

    void download_sdk(const fs::path& steamworks_dir, const std::string_view& version) {
        const auto download_url = std::format(
            "https://github.com/acidicoala/cdn/raw/refs/heads/main/valve/steamworks_sdk_{}.zip",
            version
        );

        const auto zip_file_path = fs::temp_directory_path() / (generate_random_string() + ".zip");

        kb::http_client::download_file(download_url, zip_file_path);

        try {
            const auto unzip_dir = steamworks_dir / version;
            unzip_sdk(zip_file_path, unzip_dir);
        } catch(std::exception& e) {
            std::cerr << "Unzip error: " << e.what() << std::endl;
        }

        fs::remove(zip_file_path);
    }
} // namespace

/**
 * A tool for downloading Steamworks SDK and unpacking its headers and binaries
 * for further processing by other tools.
 */
int MAIN(const int argc, const TCHAR* argv[]) { // NOLINT(*-use-internal-linkage)
    if(argc == 1) {
        print_help();
        return 0;
    }

    const auto steamworks_dir = std::filesystem::current_path() / "steamworks";

    // Special case. If there is a directory with a bunch of SDKs downloaded,
    // then we can just provide it as a single argument
    if(argc == 2) {
        if(const auto cdn_dir = kb::str::to_str(argv[1]); fs::is_directory(cdn_dir)) {
            for(const auto& entry : fs::directory_iterator(cdn_dir)) {
                const auto filename = kb::path::to_str(entry.path().filename());
                const std::regex re(R"(steamworks_sdk_(.+)\.zip)");

                if(std::smatch match; std::regex_match(filename, match, re)) {
                    if(match.size() > 1) {
                        const auto& version = match[1].str();
                        unzip_sdk(entry.path(), steamworks_dir / version);
                    }
                }
            }

            return 0;
        }
    }

    for(auto i = 1; i < argc; i++) {
        const auto version = kb::str::to_str(argv[i]);

        try {
            download_sdk(steamworks_dir, version);
        } catch(const std::exception& e) {
            LOG_ERROR("Error downloading SDK '{}'. Reason: {}", version, e.what());
        }
    }
}
