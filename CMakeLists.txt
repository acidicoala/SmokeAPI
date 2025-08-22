cmake_minimum_required(VERSION 3.24)

project(SmokeAPI VERSION 3.0.0)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox EXCLUDE_FROM_ALL)

set_32_and_64(STEAMAPI_DLL steam_api)
set_32_and_64(STEAMCLIENT_DLL steamclient)
set_32_and_64(VSTDLIB_DLL vstdlib_s)

configure_version_resource("Free DLC for everyone ʕ ᵔᴥᵔʔ")

# Setup linker exports

set_32_and_64(STEAM_API_DLL steam_api.dll steam_api64.dll)

set(
    STEAM_API_EXPORTS
    src/game_mode/exports/steam_api.cpp
    src/game_mode/exports/steam_api_flat.cpp
    src/game_mode/exports/steam_api_internal.cpp
    src/game_mode/exports/steam_api_unversioned.cpp
)

configure_build_config(extra_build_config)

set(
    SMOKE_API_SOURCES
    src/exports/steamclient.cpp
    src/exports/steamclient.hpp
    src/exports/steam_api.cpp
    src/exports/steam_api_flat.cpp
    src/exports/steam_api_internal.cpp
    src/exports/steam_api_unversioned.cpp
    src/virtuals/isteamapps.cpp
    src/virtuals/isteamclient.cpp
    src/virtuals/isteaminventory.cpp
    src/virtuals/isteamuser.cpp
    src/virtuals/steam_api_virtuals.hpp
    src/smoke_api/cache.cpp
    src/smoke_api/cache.hpp
    src/smoke_api/config.cpp
    src/smoke_api/config.hpp
    src/smoke_api/smoke_api.cpp
    src/smoke_api/smoke_api.hpp
    src/smoke_api/api.cpp
    src/smoke_api/api.hpp
    src/smoke_api/globals.cpp
    src/smoke_api/globals.hpp
    src/smoke_api/types.cpp
    src/smoke_api/types.hpp
    src/steam_interface/steam_apps.cpp
    src/steam_interface/steam_apps.hpp
    src/steam_interface/steam_client.cpp
    src/steam_interface/steam_client.hpp
    src/steam_interface/steam_interface.cpp
    src/steam_interface/steam_interface.hpp
    src/steam_interface/steam_inventory.cpp
    src/steam_interface/steam_inventory.hpp
    src/steam_interface/steam_user.cpp
    src/steam_interface/steam_user.hpp
    src/main.cpp
    ${GENERATED_LINKER_EXPORTS}
)

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES} ${VERSION_RESOURCE})

# There is a weird MSVC bug where c++23 features are not enabled in x64 builds,
# while they are available in x86. I've no idea what causes this discrepancy,
# but manually setting the MSVC compiler option fixes this issue.
target_compile_options(SmokeAPI PRIVATE /std:c++latest)

target_include_directories(SmokeAPI PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")

configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_steam_api"
    FORWARDED_DLL "${STEAMAPI_DLL}_o"
    INPUT_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/exports"
    DLL_FILES_GLOB "${CMAKE_CURRENT_SOURCE_DIR}/res/steamworks/*/binaries/${STEAM_API_DLL}"
)

configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_version"
    FORWARDED_DLL "C:/Windows/System32/version.dll"
    INPUT_SOURCES_DIR ""
    DLL_FILES_GLOB "C:/Windows/System32/version.dll"
)

configure_output_name(${STEAMAPI_DLL})

configure_include_directories()

target_link_libraries(SmokeAPI PRIVATE KoalaBox)

set(B_PRODUCTION_MODE ON)
CPMAddPackage("gh:batterycenter/embed@1.2.19")
b_embed(SmokeAPI "res/interface_lookup.json")

add_subdirectory(tools)
