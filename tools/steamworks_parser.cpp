#include <iostream>
#include <functional>
#include <filesystem>
#include <deque>
#include <fstream>

#include <nlohmann/json.hpp>
#include <cpp-tree-sitter.h>

extern "C" const TSLanguage* tree_sitter_cpp();

namespace {
    namespace fs = std::filesystem;
    using json = nlohmann::json;

    std::string_view trim(const std::string_view& s) {
        const auto start =
            std::ranges::find_if_not(s, [](const unsigned char ch) { return std::isspace(ch); });
        const auto end =
            std::find_if_not(
                s.rbegin(), s.rend(),
                [](const unsigned char ch) { return std::isspace(ch); }
            ).base();
        return start < end ? std::string_view(start, end) : std::string_view();
    }

    std::string_view unquote_if_quoted(const std::string_view& s) {
        if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"'))) {
            return s.substr(1, s.size() - 2);
        }

        // Not a plain quoted string (might be raw/user-defined). Return as-is.
        return s;
    }

    enum class VisitResult {
        Continue,
        SkipChildren,
        Stop
    };

    void walk(
        const ts::Node& root,
        const std::function<VisitResult(ts::Node)>& visit
    ) {
        // DFS traversal
        std::deque<ts::Node> queue;
        queue.push_back(root);
        auto first_visit = true;

        while (!queue.empty()) {
            const auto node = queue.front();
            queue.pop_front();

            switch (first_visit ? VisitResult::Continue : visit(node)) {
            case VisitResult::Continue:
                break;
            case VisitResult::SkipChildren:
                continue;
            case VisitResult::Stop:
                return;
            }

            for (uint32_t i = 0, count = node.getNumNamedChildren(); i < count; ++i) {
                if (const auto child = node.getNamedChild(i); !child.isNull()) {
                    queue.push_back(child);
                }
            }

            first_visit = false;
        }
    }

    void parse_header(const std::string_view& source, json& lookup) {
        const auto language = ts::Language(tree_sitter_cpp());
        auto parser = ts::Parser(language);
        const auto tree = parser.parseString(source);
        const auto root = tree.getRootNode();

        json current_lookup = {};
        std::string interface_version;

        walk(root, [&](const auto& current_node) {
            const auto current_type = current_node.getType();
            if (current_type == "class_specifier") {

                std::string interface_name;
                [[maybe_unused]] int vt_idx = 0;

                walk(current_node, [&](const ts::Node& class_node) {
                    const auto type = class_node.getType();
                    const auto value = class_node.getSourceRange(source);

                    if (type == "type_identifier" && interface_name.empty()) {
                        interface_name = value;
                        std::cout << "  Parsing interface: " << interface_name << std::endl;

                        return VisitResult::Continue;
                    }

                    if (type == "field_declaration" && value.starts_with("virtual ")) {
                        if (value.starts_with("virtual ")) {
                            walk(class_node, [&](const ts::Node& decl_node) {
                                if (decl_node.getType() == "field_identifier") {
                                    const auto function_name =
                                        decl_node.getSourceRange(source);

                                    current_lookup[function_name]["vt_idx"] = vt_idx++;
                                    return VisitResult::Stop;
                                }
                                return VisitResult::Continue;
                            });
                        }

                        return VisitResult::SkipChildren;
                    }

                    return VisitResult::Continue;
                });
            } else if (current_type == "preproc_def") {
                walk(current_node, [&](const ts::Node& preproc_node) {
                    if (preproc_node.getType() == "identifier") {
                        const auto identifier = preproc_node.getSourceRange(source);

                        return identifier.ends_with("INTERFACE_VERSION")
                                   ? VisitResult::Continue
                                   : VisitResult::Stop;
                    }

                    if (preproc_node.getType() == "preproc_arg") {
                        const auto quoted_version = preproc_node.getSourceRange(source);
                        interface_version = unquote_if_quoted(trim(quoted_version));
                        std::cout << "  Interface version: " << interface_version << std::endl;

                        return VisitResult::Stop;
                    }

                    return VisitResult::Continue;
                });
            } else if (current_type == "translation_unit" || current_type == "preproc_ifdef") {
                return VisitResult::Continue;
            }

            return VisitResult::SkipChildren;
        });

        // Save the findings
        if (!interface_version.empty()) {
            lookup[interface_version] = current_lookup;
        }
    }

    void parse_sdk(const fs::path& sdk_path, json& lookup) {
        const auto headers_dir = sdk_path / "headers";

        if (!fs::exists(headers_dir)) {
            std::cout << "Warning: SDK missing 'headers' directory: " << headers_dir << std::endl;
            return;
        }

        // Go over each file in headers directory
        for (const auto& entry : fs::directory_iterator(headers_dir)) {
            if (
                const auto& header_path = entry.path();
                header_path.extension() == ".h"
            ) {
                std::cout << "Parsing header: " << header_path << std::endl;

                // Read file as text
                std::ifstream in(header_path, std::ios::binary);
                const std::string header_contents(std::istreambuf_iterator{in}, {});

                // Parse it
                parse_header(header_contents, lookup);
            }
        }
    }

    void generate_lookup_json(const fs::path& steamworks_dir) {
        json lookup;

        // Go over each steamworks sdk version
        for (const auto& entry : fs::directory_iterator(steamworks_dir)) {
            if (!entry.is_directory()) {
                continue;
            }

            parse_sdk(entry.path(), lookup);
        }

        std::ofstream lookup_output("interface_lookup.json");
        lookup_output << std::setw(4) << lookup << std::endl;
    }
}

/**
 * A tool for parsing Steamworks headers and generating a lookup map of its interfaces.
 */
int main() {
    try {
        const auto steamworks_dir = fs::path("steamworks");

        if (!fs::exists(steamworks_dir)) {
            throw std::exception("Expected to find 'steamworks' in current working directory.");
        }

        generate_lookup_json(steamworks_dir);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}