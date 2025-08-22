#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>

#include <BS_thread_pool.hpp>
#include <cpp-tree-sitter.h>
#undef ERROR // Workaround for the ERROR enum defined in simplecpp
#include <simplecpp.h>

#include <koalabox/io.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/parser.hpp>
#include <koalabox/str.hpp>
#include <koalabox/util.hpp>

namespace {
    namespace fs = std::filesystem;
    namespace kb = koalabox;

    std::string_view unquote_if_quoted(const std::string_view& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            return s.substr(1, s.size() - 2);
        }

        // Not a plain quoted string (might be raw/user-defined). Return as-is.
        return s;
    }

    void parse_header(const std::string_view& source, nlohmann::ordered_json& lookup) {
        const auto tree = kb::parser::parse_source(source);
        const auto root = tree.getRootNode();

        nlohmann::ordered_json current_lookup = {};
        std::string interface_version;

        kb::parser::walk(root, [&](const auto& current_node) {
            const auto current_type = current_node.getType();
            const auto current_value = current_node.getSourceRange(source);
            const auto current_sexpr = current_node.getSExpr();

            if (current_type == "class_specifier") {
                std::string interface_name;
                [[maybe_unused]] int vt_idx = 0;

                kb::parser::walk(current_node, [&](const ts::Node& class_node) {
                    const auto type = class_node.getType();
                    const auto value = class_node.getSourceRange(source);

                    if (type == "type_identifier" && interface_name.empty()) {
                        interface_name = value;
                        LOG_DEBUG("Found interface: {}", interface_name);

                        return kb::parser::visit_result::Continue;
                    }

                    if (type == "field_declaration" && value.starts_with("virtual ")) {
                        if (value.starts_with("virtual ")) {
                            kb::parser::walk(class_node, [&](const ts::Node& decl_node) {
                                if (decl_node.getType() == "field_identifier") {
                                    const auto function_name = decl_node.getSourceRange(source);

                                    // Note: This doesn't take into account overloaded functions.
                                    // However, so far this project hasn't had any need to hook such
                                    // functions. Hence, no fixes have been implemented so far.

                                    current_lookup[function_name] = vt_idx++;
                                    return kb::parser::visit_result::Stop;
                                }
                                return kb::parser::visit_result::Continue;
                            });
                        }

                        return kb::parser::visit_result::SkipChildren;
                    }

                    return kb::parser::visit_result::Continue;
                });
            } else if (current_type == "preproc_def") {
                kb::parser::walk(current_node, [&](const ts::Node& preproc_node) {
                    if (preproc_node.getType() == "identifier") {
                        const auto identifier = preproc_node.getSourceRange(source);

                        return identifier.ends_with("INTERFACE_VERSION")
                                   ? kb::parser::visit_result::Continue
                                   : kb::parser::visit_result::Stop;
                    }

                    if (preproc_node.getType() == "preproc_arg") {
                        const auto quoted_version = preproc_node.getSourceRange(source);
                        const auto trimmed_version = koalabox::str::trim(quoted_version);
                        interface_version = unquote_if_quoted(trimmed_version);
                        LOG_DEBUG("Interface version: {}", interface_version);

                        return kb::parser::visit_result::Stop;
                    }

                    return kb::parser::visit_result::Continue;
                });
            } else if (current_type == "translation_unit" || current_type == "preproc_ifdef") {
                return kb::parser::visit_result::Continue;
            }

            return kb::parser::visit_result::SkipChildren;
        });

        // Save the findings
        static std::mutex mutex;
        if (not interface_version.empty()) {
            const std::lock_guard lock(mutex);
            lookup[interface_version] = current_lookup;
        }
    }

    /**
     * Turns out that preprocessing headers using a proper C++ preprocessor
     * significantly complicates AST parsing down the line.
     * To make matters worse, it also removes macro definitions with interface version string.
     * Hence, this function should not be used in practice.
     * It remains here for reference purposes only.
     */
    [[maybe_unused]] std::string preprocess_header(const fs::path& header_path) {
        auto files = std::vector<std::string>();

        static simplecpp::DUI dui{};
        dui.removeComments = true;
        // For stdlib headers
        dui.includePaths.emplace_back("dummy/");
        // For headers includes via "steam/*" path
        dui.includePaths.emplace_back(header_path.parent_path().parent_path().string());
        dui.defines.emplace_back("VALVE_CALLBACK_PACK_LARGE");

        simplecpp::OutputList output_list;
        simplecpp::TokenList raw_token_list(header_path.string(), files, &output_list);
        raw_token_list.removeComments();

        simplecpp::FileDataCache cache;
        simplecpp::TokenList output_token_list(files);
        simplecpp::preprocess(output_token_list, raw_token_list, files, cache, dui, &output_list);
        simplecpp::cleanup(cache);

        for (const simplecpp::Output& output : output_list) {
            if (output.type == simplecpp::Output::MISSING_HEADER) {
                LOG_WARN(
                    "Place missing empty header at: " + fs::absolute(fs::path("dummy")).string()
                );
            }

            const auto msg = std::format(
                "msg = {}, line={}, col={}, type = {}",
                output.msg,
                output.location.line,
                output.location.col,
                static_cast<int>(output.type)
            );

            throw std::runtime_error(msg);
        }

        return output_token_list.stringify();
    }

    /**
     * Certain Steam macros break C++ AST parser, if left unprocessed.
     * This function does that in a very naive manner. Stupid, but works.
     */
    std::string manually_preprocess_header(const fs::path& header_path) {
        const auto header_contents = kb::io::read_file(header_path);

        // language=RegExp
        const std::regex re(R"(STEAM_PRIVATE_API\s*\(\s*([^)]+)\s*\))");
        const auto processed_contents = std::regex_replace(header_contents, re, "$1");

        return processed_contents;
    }

    void
    parse_sdk(const fs::path& sdk_path, nlohmann::ordered_json& lookup, BS::thread_pool<>& pool) {
        const auto headers_dir = sdk_path / "headers\\steam";

        if (not fs::exists(headers_dir)) {
            LOG_WARN("Warning: SDK missing 'headers' directory: {}", headers_dir.string());
            return;
        }

        LOG_INFO("Parsing SDK: {}", headers_dir.string());

        // Go over each file in headers directory
        for (const auto& entry : fs::directory_iterator(headers_dir)) {
            if (const auto& header_path = entry.path(); header_path.extension() == ".h") {
                const auto task = pool.submit_task([&, header_path] {
                    try {
                        LOG_DEBUG("Parsing header: {}", header_path.string());

                        const auto processed_header = manually_preprocess_header(header_path);
                        parse_header(processed_header, lookup);
                    } catch (std::exception& e) {
                        LOG_CRITICAL(e.what());
                        exit(-1);
                    }
                });
            }
        }
    }

    void generate_lookup_json(
        const fs::path& steamworks_dir, //
        const std::set<std::string>& sdk_filter
    ) {
        nlohmann::ordered_json lookup;

        // The thread pool noticeably speeds up the overall parsing.
        // Thread count of 4 seems to yield most optimal performance benefits.
        constexpr auto thread_count = 4;
        LOG_INFO("Creating task pool with {} threads", thread_count);
        BS::thread_pool pool(thread_count);

        // Go over each steamworks sdk version
        for (const auto& entry : fs::directory_iterator(steamworks_dir)) {
            if (not entry.is_directory()) {
                continue;
            }

            if (not sdk_filter.empty() and
                not sdk_filter.contains(entry.path().filename().string())) {
                continue;
            }

            parse_sdk(entry.path(), lookup, pool);
        }

        // Wait for all tasks to finish
        pool.wait();

        const auto interface_lookup_path = fs::path("interface_lookup.json");

        std::ofstream lookup_output(interface_lookup_path);
        lookup_output << std::setw(4) << lookup;

        LOG_INFO("Interface lookup generated at: {}", fs::absolute(interface_lookup_path).string());
    }
}

/**
 * A tool for parsing Steamworks headers and generating a lookup map of its interfaces.
 * Optionally accepts a list of folder names that filters which sdk versions will be parsed.
 * No list means all versions will be parsed.
 */
int wmain(const int argc, const wchar_t* argv[]) { // NOLINT(*-use-internal-linkage)
    try {
        koalabox::logger::init_console_logger();

        std::set<std::string> sdk_filter;
        if (argc > 1) {
            for (auto i = 1; i < argc; i++) {
                const auto version = koalabox::str::to_str(argv[i]);
                sdk_filter.insert(version);
            }
        }

        const auto steamworks_dir = fs::path("steamworks");

        if (!fs::exists(steamworks_dir)) {
            throw std::exception("Expected to find 'steamworks' in current working directory.");
        }

        const auto start = std::chrono::steady_clock::now();
        generate_lookup_json(steamworks_dir, sdk_filter);
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = duration_cast<std::chrono::seconds>(end - start);

        LOG_INFO("Finished parsing steamworks in {} seconds", elapsed.count());
    } catch (std::exception& e) {
        LOG_CRITICAL("Error: {}", e.what());
        return 1;
    }
}