cmake_minimum_required(VERSION 3.24)

project(smoke-api-tools LANGUAGES CXX)

### Install CPM package manager

file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake
    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

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

CPMAddPackage(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

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

### Install ZIP library https://github.com/richgel999/miniz
CPMAddPackage(
    NAME miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG c883286f1a6443720e7705450f59e579a4bbb8e2
)

### Steamworks Downloader executable

add_executable(steamworks_downloader steamworks_downloader.cpp)
target_link_libraries(steamworks_downloader PRIVATE
    cpr # HTTP client
    miniz # ZIP library
)

### Steamworks Parser executable

add_executable(steamworks_parser steamworks_parser.cpp)
target_link_libraries(steamworks_parser PRIVATE
    nlohmann_json   # JSON parser
    tree-sitter-cpp # C++ grammar
    cpp-tree-sitter # C++ bindings for tree-sitter
)
