cmake_minimum_required(VERSION 3.24)

project(smoke-api-tools LANGUAGES CXX)

### Thread pool library

## https://github.com/bshoshany/thread-pool
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
target_link_libraries(steamworks_downloader PRIVATE KoalaBox)

### Steamworks Parser executable

add_executable(steamworks_parser steamworks_parser.cpp)
target_link_libraries(steamworks_parser PRIVATE
    KoalaBox
    BS_thread_pool
)
