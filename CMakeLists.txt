cmake_minimum_required(VERSION 3.24)

project(SmokeAPI VERSION 3.0.0)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox EXCLUDE_FROM_ALL)
add_subdirectory(tools)

set_32_and_64(STEAMAPI_DLL steam_api)
set_32_and_64(STEAMCLIENT_DLL steamclient)
set_32_and_64(STEAM_API_DLL steam_api.dll steam_api64.dll)

configure_build_config(extra_build_config)

set(SMOKE_API_STATIC_SOURCES
    static/smoke_api/interfaces/steam_apps.hpp
    static/smoke_api/interfaces/steam_apps.cpp
    static/smoke_api/interfaces/steam_inventory.hpp
    static/smoke_api/interfaces/steam_inventory.cpp
    static/smoke_api/interfaces/steam_user.hpp
    static/smoke_api/interfaces/steam_user.cpp
    static/smoke_api/api.hpp
    static/smoke_api/api.cpp
    static/smoke_api/cache.hpp
    static/smoke_api/cache.cpp
    static/smoke_api/config.hpp
    static/smoke_api/config.cpp
    static/smoke_api/types.hpp
    static/smoke_api/types.cpp
    static/smoke_api/steamclient/steamclient.hpp
)

set(SMOKE_API_SOURCES
    ${SMOKE_API_STATIC_SOURCES}
    src/steam_api/exports/steam_api.cpp
    src/steam_api/exports/steam_api_flat.cpp
    src/steam_api/exports/steam_api_internal.cpp
    src/steam_api/exports/steam_api_unversioned.cpp
    src/steam_api/virtuals/isteamapps.cpp
    src/steam_api/virtuals/isteamclient.cpp
    src/steam_api/virtuals/isteaminventory.cpp
    src/steam_api/virtuals/isteamuser.cpp
    src/steam_api/virtuals/steam_api_virtuals.hpp
    src/steam_api/steam_client.hpp
    src/steam_api/steam_client.cpp
    src/steam_api/steam_interface.cpp
    src/steam_api/steam_interface.hpp
    src/steamclient.cpp
    src/main.cpp
    src/smoke_api.cpp
    src/smoke_api.hpp
)

### SmokeAPI interface

add_library(SmokeAPI_interface INTERFACE)

# There is a weird MSVC bug where c++23 features are not enabled in x64 builds,
# while they are available in x86. I've no idea what causes this discrepancy,
# but manually setting the MSVC compiler option fixes this issue.
#target_compile_options(SmokeAPI_interface PUBLIC /std:c++latest)

target_include_directories(SmokeAPI_interface INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/static>"
)
target_link_libraries(SmokeAPI_interface INTERFACE KoalaBox $<TARGET_OBJECTS:KoalaBox>)


### Static SmokeAPI

add_library(SmokeAPI_static STATIC ${SMOKE_API_STATIC_SOURCES})
target_link_libraries(SmokeAPI_static PUBLIC SmokeAPI_interface)
#target_include_directories(SmokeAPI_static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/static")

### Shared SmokeAPI

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES})
target_link_libraries(SmokeAPI PUBLIC SmokeAPI_interface)
set_target_properties(SmokeAPI PROPERTIES RUNTIME_OUTPUT_NAME ${STEAMAPI_DLL})
configure_version_resource(SmokeAPI "Free DLC for everyone ʕ ᵔᴥᵔʔ")
target_include_directories(SmokeAPI PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
#    "${CMAKE_CURRENT_SOURCE_DIR}/static"
    "${CMAKE_CURRENT_BINARY_DIR}")

CPMAddPackage(
    URI "gh:batterycenter/embed@1.2.19"
    OPTIONS "B_PRODUCTION_MODE ON"
)
b_embed(SmokeAPI "res/interface_lookup.json")

configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_steam_api"
    FORWARDED_DLL "${STEAMAPI_DLL}_o"
    INPUT_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/steam_api/exports"
    DLL_FILES_GLOB "${CMAKE_CURRENT_SOURCE_DIR}/res/steamworks/*/binaries/${STEAM_API_DLL}"
)

# TODO: Why not add other compatible DLLs?
configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_version"
    FORWARDED_DLL "C:/Windows/System32/version.dll"
    INPUT_SOURCES_DIR ""
    DLL_FILES_GLOB "C:/Windows/System32/version.dll"
)
