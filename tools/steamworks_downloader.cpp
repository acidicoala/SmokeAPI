#include <iostream>
#include <filesystem>
#include <random>
#include <string>

#include <cpr/cpr.h>
#include <miniz.h>

namespace {
    namespace fs = std::filesystem;

    // Function to extract files from a ZIP archive that match multiple regex patterns
    void extract_files(
        const fs::path& zip_path,
        const std::function<fs::path(const std::string& name, bool is_dir)>& predicate
    ) {
        mz_zip_archive zip = {};

        if (!mz_zip_reader_init_file(&zip, zip_path.string().c_str(), 0)) {
            throw std::runtime_error("mz_zip_reader_init_file() failed for: " + zip_path.string());
        }

        [[maybe_unused]] auto guard = [&zip] { mz_zip_reader_end(&zip); };
        try {
            std::error_code ec;
            const mz_uint num_files = mz_zip_reader_get_num_files(&zip);

            for (mz_uint i = 0; i < num_files; ++i) {
                mz_zip_archive_file_stat st;
                if (!mz_zip_reader_file_stat(&zip, i, &st)) {
                    mz_zip_reader_end(&zip);
                    throw std::runtime_error(
                        "mz_zip_reader_file_stat() failed at index " + std::to_string(i));
                }

                const std::string name = st.m_filename;

                // Skip dangerous names early
                if (name.empty()) {
                    continue;
                }

                const bool is_dir = mz_zip_reader_is_file_a_directory(&zip, i) != 0;

                const auto out_path = predicate(name, is_dir);
                if (out_path.empty()) {
                    continue;
                }

                if (is_dir) {
                    fs::create_directories(out_path, ec);
                    if (ec) {
                        mz_zip_reader_end(&zip);
                        throw std::runtime_error(
                            "Failed to create directory: " + out_path.string() + " (" + ec.message()
                            + ")");
                    }
                    continue;
                }

                // Ensure parent directories exist
                fs::create_directories(out_path.parent_path(), ec);
                if (ec) {
                    mz_zip_reader_end(&zip);
                    throw std::runtime_error(
                        "Failed to create parent directories for: " + out_path.string() + " (" + ec.
                        message() + ")");
                }

                // Extract to heap then write to file
                size_t uncomp_size = 0;
                void* p = mz_zip_reader_extract_to_heap(&zip, i, &uncomp_size, 0);
                if (!p) {
                    mz_zip_reader_end(&zip);
                    throw std::runtime_error("Extraction failed for entry: " + name);
                }

                std::ofstream ofs(out_path, std::ios::binary);
                if (!ofs) {
                    mz_free(p);
                    mz_zip_reader_end(&zip);
                    throw std::runtime_error(
                        "Failed to open output file for writing: " + out_path.string());
                }
                ofs.write(static_cast<const char*>(p),
                          static_cast<std::streamsize>(uncomp_size));
                ofs.close();
                mz_free(p);

                if (!ofs) {
                    mz_zip_reader_end(&zip);
                    throw std::runtime_error("Failed to write output file: " + out_path.string());
                }
            }

            mz_zip_reader_end(&zip);
        } catch (...) {
            // Ensure cleanup on exceptions
            guard();
            throw;
        }
    }

    std::string generate_random_string(const size_t length) {
        static constexpr char charset[] =
            "0123456789"
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
        extract_files(zip_file_path, [&](const std::string& name, const bool) {
            if (name.starts_with("sdk/public/steam/") && name.ends_with(".h")) {
                return unzip_dir / "headers" / fs::path(name).filename();
            }

            if (
                name.starts_with("sdk/redistributable_bin/") &&
                name.ends_with(".dll") &&
                name.find("steam_api") != std::string::npos
            ) {
                return unzip_dir / "binaries" / fs::path(name).filename();
            }

            return fs::path();
        });

    }

    void download_sdk(
        const fs::path& steamworks_dir,
        const std::string_view& version
    ) {
        const auto download_url = std::format(
            "https://github.com/acidicoala/cdn/raw/refs/heads/main/valve/steamworks_sdk_{}.zip",
            version
        );

        const auto zip_file_path = fs::temp_directory_path()
                                   / (generate_random_string(16) + ".zip");

        std::cout << "Downloading " << download_url << " to " << zip_file_path << std::endl;

        std::ofstream of(zip_file_path, std::ios::binary);
        if (
            const auto res = cpr::Download(of, cpr::Url{download_url});
            res.error.code != cpr::ErrorCode::OK
        ) {
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

}

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
            std::cerr
                << std::format("Error downloading SDK '{}'. Reason: {}", argv[i], e.what())
                << std::endl;
        }
    }
}