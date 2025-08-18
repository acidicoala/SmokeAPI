cmake_minimum_required(VERSION 3.24)

project(smoke-api-tools LANGUAGES CXX)

### Install parser library

CPMAddPackage(
    NAME cpp-tree-sitter
    GIT_REPOSITORY https://github.com/nsumner/cpp-tree-sitter.git
    # The latest commit includes a fix for MSVC, which is not present in the latest release
    GIT_TAG 9cd7f3c9de9fc6d19bc982c6772fcd7a39e5797a
)

# Downloads a tree-sitter grammar from github
# and makes it available as a cmake library target.
add_grammar_from_repo(tree-sitter-cpp                  # Library name
    https://github.com/tree-sitter/tree-sitter-cpp.git # Repository URL
    0.23.4                                             # Version tag
)

### Install JSON library

#CPMAddPackage(
#    NAME nlohmann_json
#    GIT_REPOSITORY https://github.com/nlohmann/json.git
#    GIT_TAG v3.12.0
#)

### Install HTTP client library

set(CURL_USE_LIBPSL OFF)
set(CPR_USE_SYSTEM_LIB_PSL ON)
set(ENABLE_CURL_MANUAL OFF)
set(BUILD_LIBCURL_DOCS OFF)
set(BUILD_EXAMPLES OFF)
CPMAddPackage(
    NAME cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.12.0
)

CPMAddPackage(
    NAME BS_thread_pool
    GITHUB_REPOSITORY bshoshany/thread-pool
    VERSION 5.0.0
    EXCLUDE_FROM_ALL
    SYSTEM
)
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE ${BS_thread_pool_SOURCE_DIR}/include)

### Steamworks Downloader executable

add_executable(steamworks_downloader steamworks_downloader.cpp)
target_link_libraries(steamworks_downloader PRIVATE
    KoalaBox
    cpr # HTTP client
    miniz # ZIP library TODO: Use koalabox instead
)

### Steamworks Parser executable

add_executable(steamworks_parser steamworks_parser.cpp)
target_link_libraries(steamworks_parser PRIVATE
    KoalaBox
    BS_thread_pool
#    nlohmann_json   # JSON parser
#    tree-sitter-cpp # C++ grammar
#    cpp-tree-sitter # C++ bindings for tree-sitter
)
