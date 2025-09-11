cmake_minimum_required(VERSION 3.24)

project(SmokeAPI VERSION 3.1.1)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox)
add_subdirectory(tools)

set_32_and_64(STEAMAPI_DLL steam_api)
set_32_and_64(STEAMCLIENT_DLL steamclient)
set_32_and_64(STEAM_API_DLL steam_api steam_api64)
set_32_and_64(SMOKEAPI_DLL SmokeAPI32 SmokeAPI64)

configure_build_config(extra_build_config)

set(SMOKE_API_STATIC_SOURCES
    static/smoke_api/interfaces/steam_apps.hpp
    static/smoke_api/interfaces/steam_apps.cpp
    static/smoke_api/interfaces/steam_http.hpp
    static/smoke_api/interfaces/steam_http.cpp
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
    src/smoke_api/smoke_api.cpp
    src/smoke_api/smoke_api.hpp
    src/steam_api/virtuals/isteamapps.cpp
    src/steam_api/virtuals/isteamclient.cpp
    src/steam_api/virtuals/isteamgameserver.cpp
    src/steam_api/virtuals/isteamhttp.cpp
    src/steam_api/virtuals/isteaminventory.cpp
    src/steam_api/virtuals/isteamuser.cpp
    src/steam_api/virtuals/steam_api_virtuals.hpp
    src/steam_api/steam_client.hpp
    src/steam_api/steam_client.cpp
    src/steam_api/steam_interfaces.cpp
    src/steam_api/steam_interfaces.hpp
    src/steamclient/steamclient.cpp
    src/main.cpp
)

### SmokeAPI interface

add_library(SmokeAPI_common INTERFACE)
add_library(SmokeAPI::common ALIAS SmokeAPI_common)

target_include_directories(SmokeAPI_common INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/static>"
)
target_link_libraries(SmokeAPI_common INTERFACE KoalaBox $<TARGET_OBJECTS:KoalaBox>)


### Static SmokeAPI

add_library(SmokeAPI_static STATIC ${SMOKE_API_STATIC_SOURCES})
add_library(SmokeAPI::static ALIAS SmokeAPI_static)

target_link_libraries(SmokeAPI_static PUBLIC SmokeAPI::common)

### Shared SmokeAPI

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES} ${VERSION_RESOURCE})
target_link_libraries(SmokeAPI PUBLIC SmokeAPI::common)
set_target_properties(SmokeAPI PROPERTIES RUNTIME_OUTPUT_NAME ${SMOKEAPI_DLL})
configure_version_resource(
    TARGET SmokeAPI
    FILE_DESC "Steamworks DLC unlocker"
    ORIG_NAME ${SMOKEAPI_DLL}
)
configure_include_directories()

## https://github.com/batterycenter/embed
CPMAddPackage(
    URI "gh:batterycenter/embed@1.2.19"
    OPTIONS "B_PRODUCTION_MODE ON"
)
b_embed(SmokeAPI "res/interface_lookup.json")

configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_steam_api"
    FORWARDED_DLL "${STEAMAPI_DLL}_o"
    INPUT_SOURCES_DIR ""
    DLL_FILES_GLOB "${CMAKE_CURRENT_SOURCE_DIR}/res/steamworks/*/binaries/${STEAM_API_DLL}.dll"
)

configure_linker_exports(
    TARGET SmokeAPI
    HEADER_NAME "linker_exports_for_version"
    FORWARDED_DLL "C:/Windows/System32/version.dll"
    INPUT_SOURCES_DIR ""
    DLL_FILES_GLOB "C:/Windows/System32/version.dll"
)
