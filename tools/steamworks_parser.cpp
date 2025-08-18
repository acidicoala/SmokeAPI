#include "koalabox/crypto.hpp"
#include "koalabox/io.hpp"
#include "koalabox/util.hpp"

#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>

#include <BS_thread_pool.hpp>
#include <cpp-tree-sitter.h>
#include <nlohmann/json.hpp>

#include <koalabox/logger.hpp>
#include <koalabox/parser.hpp>
#include <koalabox/str.hpp>

namespace {
    using json = nlohmann::json;
    namespace fs = std::filesystem;
    namespace parser = koalabox::parser;

    std::string_view unquote_if_quoted(const std::string_view& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            return s.substr(1, s.size() - 2);
        }

        // Not a plain quoted string (might be raw/user-defined). Return as-is.
        return s;
    }

    void parse_header(const std::string_view& source, json& lookup) {
        const auto tree = parser::parse_source(source);
        const auto root = tree.getRootNode();

        json current_lookup = {};
        std::string interface_version;

        parser::walk(root, [&](const auto& current_node) {
            const auto current_type = current_node.getType();
            if (current_type == "class_specifier") {
                std::string interface_name;
                [[maybe_unused]] int vt_idx = 0;

                koalabox::parser::walk(current_node, [&](const ts::Node& class_node) {
                    const auto type = class_node.getType();
                    const auto value = class_node.getSourceRange(source);

                    if (type == "type_identifier" && interface_name.empty()) {
                        interface_name = value;
                        LOG_DEBUG("Found interface: {}", interface_name);

                        return parser::visit_result::Continue;
                    }

                    if (type == "field_declaration" && value.starts_with("virtual ")) {
                        if (value.starts_with("virtual ")) {
                            koalabox::parser::walk(class_node, [&](const ts::Node& decl_node) {
                                if (decl_node.getType() == "field_identifier") {
                                    const auto function_name = decl_node.getSourceRange(source);

                                    current_lookup[function_name] = vt_idx++;
                                    return parser::visit_result::Stop;
                                }
                                return parser::visit_result::Continue;
                            });
                        }

                        return parser::visit_result::SkipChildren;
                    }

                    return parser::visit_result::Continue;
                });
            } else if (current_type == "preproc_def") {
                koalabox::parser::walk(current_node, [&](const ts::Node& preproc_node) {
                    if (preproc_node.getType() == "identifier") {
                        const auto identifier = preproc_node.getSourceRange(source);

                        return identifier.ends_with("INTERFACE_VERSION")
                                   ? parser::visit_result::Continue
                                   : parser::visit_result::Stop;
                    }

                    if (preproc_node.getType() == "preproc_arg") {
                        const auto quoted_version = preproc_node.getSourceRange(source);
                        const auto trimmed_version = koalabox::str::trim(quoted_version);
                        interface_version = unquote_if_quoted(trimmed_version);
                        LOG_DEBUG("Interface version: {}", interface_version);

                        return parser::visit_result::Stop;
                    }

                    return parser::visit_result::Continue;
                });
            } else if (current_type == "translation_unit" || current_type == "preproc_ifdef") {
                return parser::visit_result::Continue;
            }

            return parser::visit_result::SkipChildren;
        });

        // Save the findings
        static std::mutex mutex;
        if (not interface_version.empty()) {
            const std::lock_guard lock(mutex);
            lookup[interface_version] = current_lookup;
        }
    }

    void parse_sdk(const fs::path& sdk_path, json& lookup, BS::thread_pool<>& pool) {
        const auto headers_dir = sdk_path / "headers";

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

                        // Read file as text
                        const auto header_contents = koalabox::io::read_file(header_path);

                        parse_header(header_contents, lookup);
                    } catch (std::exception& e) {
                        koalabox::util::panic(std::format("Error parsing header: {}", e.what()));
                    }
                });
            }
        }
    }

    void generate_lookup_json(const fs::path& steamworks_dir) {
        json lookup;

        // Create the thread pool
        const auto thread_count = std::max(2U, std::thread::hardware_concurrency() / 2);
        LOG_INFO("Creating task pool with {} threads", thread_count);
        BS::thread_pool pool(thread_count);

        // Go over each steamworks sdk version
        for (const auto& entry : fs::directory_iterator(steamworks_dir)) {
            if (not entry.is_directory()) {
                continue;
            }

            parse_sdk(entry.path(), lookup, pool);
        }

        // Wait for all tasks to finish
        pool.wait();

        std::ofstream lookup_output("interface_lookup.json");
        lookup_output << std::setw(4) << lookup << std::endl;
    }
}

/**
 * A tool for parsing Steamworks headers and generating a lookup map of its interfaces.
 */
int main() {
    try {
        koalabox::logger::init_console_logger();

        const auto steamworks_dir = fs::path("steamworks");

        if (!fs::exists(steamworks_dir)) {
            throw std::exception("Expected to find 'steamworks' in current working directory.");
        }

        const auto start = std::chrono::steady_clock::now();
        generate_lookup_json(steamworks_dir);
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = duration_cast<std::chrono::seconds>(end - start);

        LOG_INFO("Finished parsing steamworks in {} seconds", elapsed.count());
    } catch (std::exception& e) {
        LOG_CRITICAL("Error: {}", e.what());
        return 1;
    }
}