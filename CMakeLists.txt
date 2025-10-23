cmake_minimum_required(VERSION 3.24)

project(SmokeAPI VERSION 4.0.0)

include(KoalaBox/cmake/KoalaBox.cmake)
add_subdirectory(KoalaBox)
add_subdirectory(tools)

set(SMOKE_API_STATIC_SOURCES
    static/smoke_api/interfaces/steam_apps.hpp
    static/smoke_api/interfaces/steam_apps.cpp
    static/smoke_api/interfaces/steam_http.hpp
    static/smoke_api/interfaces/steam_http.cpp
    static/smoke_api/interfaces/steam_inventory.hpp
    static/smoke_api/interfaces/steam_inventory.cpp
    static/smoke_api/interfaces/steam_user.hpp
    static/smoke_api/interfaces/steam_user.cpp
    static/smoke_api/interfaces/steam_user_stats.hpp
    static/smoke_api/interfaces/steam_user_stats.cpp
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

if(IS_LINUX)
    set(GENERATED_SOURCES_DIR src/generated/${BITNESS})
    file(MAKE_DIRECTORY ${GENERATED_SOURCES_DIR})
    set(GENERATED_SOURCES
        ${GENERATED_SOURCES_DIR}/proxy_exports.hpp
        ${GENERATED_SOURCES_DIR}/proxy_exports.cpp
    )

    foreach(SRC IN LISTS GENERATED_SOURCES)
        file(TOUCH ${SRC})
    endforeach()
endif()

set(SMOKE_API_SOURCES
    ${SMOKE_API_STATIC_SOURCES}
    ${GENERATED_SOURCES}
    src/smoke_api/smoke_api.cpp
    src/smoke_api/smoke_api.hpp
    src/steam_api/steam_api_flat.cpp
    src/steam_api/virtuals/isteamapps.cpp
    src/steam_api/virtuals/isteamclient.cpp
    src/steam_api/virtuals/isteamgameserver.cpp
    src/steam_api/virtuals/isteamhttp.cpp
    src/steam_api/virtuals/isteaminventory.cpp
    src/steam_api/virtuals/isteamuser.cpp
    src/steam_api/virtuals/isteamuserstats.cpp
    src/steam_api/virtuals/steam_api_virtuals.hpp
    src/steam_api/steam_client.hpp
    src/steam_api/steam_client.cpp
    src/steam_api/steam_interfaces.cpp
    src/steam_api/steam_interfaces.hpp
    src/steamclient/steamclient.cpp
)

if(WIN32)
    set_32_and_64(STEAM_API_MODULE steam_api)
    set_32_and_64(STEAMCLIENT_DLL steamclient64.dll steamclient.dll)

    list(APPEND SMOKE_API_SOURCES src/main_win.cpp)
else()
    set(STEAM_API_MODULE libsteam_api)
    set(STEAMCLIENT_DLL steamclient)

    list(APPEND SMOKE_API_SOURCES src/main_linux.cpp)
endif()
set_32_and_64(SMOKE_API_FILENAME smoke_api32 smoke_api64)

configure_build_config(extra_build_config)

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

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES})
target_link_libraries(SmokeAPI PUBLIC SmokeAPI::common)
set_target_properties(SmokeAPI PROPERTIES OUTPUT_NAME ${SMOKE_API_FILENAME})
configure_include_directories()

## https://github.com/batterycenter/embed
target_compile_definitions(SmokeAPI PUBLIC B_PRODUCTION_MODE)
CPMAddPackage(
    URI "gh:batterycenter/embed@1.2.19"
    OPTIONS "B_PRODUCTION_MODE ON"
)
b_embed(SmokeAPI "res/interface_lookup.json")

if(WIN32)
    configure_version_resource(
        TARGET SmokeAPI
        FILE_DESC "Steamworks DLC unlocker"
        ORIG_NAME ${SMOKE_API_FILENAME}
    )
    configure_linker_exports(
        TARGET SmokeAPI
        HEADER_NAME "linker_exports_for_steam_api.h"
        FORWARDED_DLL_NAME "${STEAM_API_MODULE}_o"
        LIB_FILES_GLOB "${CMAKE_CURRENT_SOURCE_DIR}/res/steamworks/*/binaries/${STEAM_API_MODULE}.dll"
        SOURCES_INPUT_PATH ""
    )

    set(HEADER_CONTENT "#pragma once\n\n")

    foreach(WIN_DLL version winhttp winmm)
        set(HEADER_NAME "linker_exports_for_${WIN_DLL}.h")
        configure_linker_exports(
            TARGET SmokeAPI
            HEADER_NAME "${HEADER_NAME}"
            FORWARDED_DLL_NAME "C:/Windows/System32/${WIN_DLL}.dll"
            LIB_FILES_GLOB "C:/Windows/System32/${WIN_DLL}.dll"
            SOURCES_INPUT_PATH ""
        )

        set(HEADER_CONTENT "${HEADER_CONTENT}#include <${HEADER_NAME}>\n")
    endforeach()

    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/linker_exports_for_windows_dlls.h"
        CONTENT "${HEADER_CONTENT}"
    )

    # Ignore linker warnings regarding COM-related private exports
    set_target_properties(SmokeAPI PROPERTIES LINK_FLAGS "/ignore:4104")
endif()
