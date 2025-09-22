#include <random>
#include <regex>

#include <elfio/elfio.hpp>
#include <glob/glob.h>

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
        LOG_INFO(
            "Steamworks SDK downloader for SmokeAPI v1.0\n"
            "Usage:   steamworks_downloader version1 version2 ... versionN\n"
            "Example: steamworks_downloader 100 158a 162\n"
            "Alternative usage: steamworks_downloader C:/path/to/downloaded_sdk/\n"
            "SDK version list available at: https://partner.steamgames.com/downloads/list"
        );
    }

    void fix_linux_binary_flags(const fs::path& bin_path) {
        const auto bin_path_str = kb::path::to_str(bin_path);

        ELFIO::elfio reader;
        if(!reader.load(bin_path_str)) {
            LOG_ERROR("Failed to read Linux binary: {}", bin_path_str);
            return;
        }

        // Iterate over program headers to find PT_GNU_STACK
        for(const auto& segment : reader.segments) {
            if(segment->get_type() != ELFIO::PT_GNU_STACK) { continue; }
            ELFIO::Elf_Xword flags = segment->get_flags();

            if(flags & ELFIO::PF_X) {
                flags &= ~ELFIO::PF_X;
                segment->set_flags(flags);

                LOG_INFO("Cleared PF_X on PT_GNU_STACK in binary: {}", bin_path_str);
            }

            break; // No need to iterate other segments
        }

        if(!reader.save(bin_path_str)) {
            LOG_ERROR("Failed to save Linux binary: {}", bin_path_str);
        }
    }

    /**
     * Some older libsteam_api.so binaries have PF_X (execute) flag from the PT_GNU_STACK enabled.
     * This prevents us from loading them on modern Linux systems. Hence, we need to clear it ourselves.
     * Affected versions (all 32-bit): 106-107
     */
    void fix_linux_binaries(const fs::path& binary_directory) {
        const auto so_files_glob = kb::path::to_str(binary_directory / "**" / "*.so");
        const auto linux_binary_list = glob::rglob(so_files_glob);

        for(const auto& bin_path : linux_binary_list) {
            if(not fs::exists(bin_path)) {
                continue;
            }

            fix_linux_binary_flags(bin_path);
        }
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

        fix_linux_binaries(unzip_dir);
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
            LOG_ERROR("Unzip error: {}", e.what());
        }

        fs::remove(zip_file_path);
    }
}

/**
 * A tool for downloading Steamworks SDK and unpacking its headers and binaries
 * for further processing by other tools.
 */
int MAIN(const int argc, const TCHAR* argv[]) { // NOLINT(*-use-internal-linkage)
    kb::logger::init_console_logger();

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
