#include <filesystem>
#include <iostream>
#include <random>
#include <string>

#include <cpr/cpr.h>

#include <koalabox/zip.hpp>

namespace {
    namespace fs = std::filesystem;
    namespace zip = koalabox::zip;

    // ReSharper disable once CppDFAConstantParameter
    std::string generate_random_string(const size_t length) {
        static constexpr char charset[] = "0123456789"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "abcdefghijklmnopqrstuvwxyz";

        thread_local std::mt19937_64 rng{std::random_device{}()};
        thread_local std::uniform_int_distribution<std::size_t> dist(0, sizeof(charset) - 2);

        std::string result;
        result.reserve(length);
        for (std::size_t i = 0; i < length; ++i) {
            result += charset[dist(rng)];
        }

        return result;
    }

    void print_help() {
        std::cout << "Steamworks SDK downloader for SmokeAPI v1.0" << std::endl
                  << "Usage:   steamworks_downloader version1 version2 ... versionN" << std::endl
                  << "Example: steamworks_downloader 100 158a 162" << std::endl
                  << "SDK version list available at: "
                  << "https://partner.steamgames.com/downloads/list" << std::endl;
    }

    void unzip_sdk(const fs::path& zip_file_path, const fs::path& unzip_dir) {
        zip::extract_files(zip_file_path, [&](const std::string& name, const bool) {
            if (name.starts_with("sdk/public/steam/") && name.ends_with(".h")) {
                return unzip_dir / "headers" / fs::path(name).filename();
            }

            if (name.starts_with("sdk/redistributable_bin/") && name.ends_with(".dll") &&
                name.find("steam_api") != std::string::npos) {
                return unzip_dir / "binaries" / fs::path(name).filename();
            }

            return fs::path();
        });
    }

    void download_sdk(const fs::path& steamworks_dir, const std::string_view& version) {
        const auto download_url = std::format(
            "https://github.com/acidicoala/cdn/raw/refs/heads/main/valve/steamworks_sdk_{}.zip",
            version
        );

        const auto zip_file_path =
            fs::temp_directory_path() / (generate_random_string(16) + ".zip");

        std::cout << "Downloading " << download_url << " to " << zip_file_path << std::endl;

        std::ofstream of(zip_file_path, std::ios::binary);
        if (const auto res = cpr::Download(of, cpr::Url{download_url});
            res.error.code != cpr::ErrorCode::OK) {
            std::cerr << "Download error: " << res.error.message << std::endl;
            return;
        }
        of.close();

        try {
            const auto unzip_dir = steamworks_dir / version;
            unzip_sdk(zip_file_path, unzip_dir);
        } catch (std::exception& e) {
            std::cerr << "Unzip error: " << e.what() << std::endl;
        }

        fs::remove(zip_file_path);
    }

} // namespace

/**
 * A tool for downloading Steamworks SDK and unpacking its headers and binaries
 * for further processing by other tools.
 */
int main(const int argc, const char** argv) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    const auto streamworks_dir = std::filesystem::current_path() / "steamworks";

    for (auto i = 1; i < argc; i++) {
        try {
            download_sdk(streamworks_dir, argv[i]);
        } catch (const std::exception& e) {
            std::cerr << std::format("Error downloading SDK '{}'. Reason: {}", argv[i], e.what())
                      << std::endl;
        }
    }
}