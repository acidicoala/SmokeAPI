cmake_minimum_required(VERSION 3.24)

project(SmokeAPI VERSION 2.0.51)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox EXCLUDE_FROM_ALL)

set_32_and_64(STEAMAPI_DLL steam_api)
set_32_and_64(STEAMCLIENT_DLL steamclient)
set_32_and_64(VSTDLIB_DLL vstdlib_s)

configure_version_resource("Free DLC for everyone ʕ ᵔᴥᵔʔ")

# Setup linker exports

set_32_and_64(DLL_SUFFIX steam_api win64/steam_api64)
file(GLOB DLL_INPUT "res/dll/*/sdk/redistributable_bin/${DLL_SUFFIX}.dll")

set(
    STEAM_API_EXPORTS
    "src/game_mode/exports/steam_api_flat.cpp"
    "src/game_mode/exports/steam_api_internal.cpp"
    "src/game_mode/exports/steam_api_unversioned.cpp"
)

configure_linker_exports(
    FORWARDED_DLL "${STEAMAPI_DLL}_o"
    INPUT_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/game_mode/exports"
    INPUT_DLLS "${DLL_INPUT}"
    DEP_SOURCES "${STEAM_API_EXPORTS}"
)

configure_build_config(extra_build_config)

set(
    SMOKE_API_SOURCES
    src/common/app_cache.cpp
    src/common/app_cache.hpp
    src/common/steamclient_exports.cpp
    src/common/steamclient_exports.hpp
    src/core/api.cpp
    src/core/api.hpp
    src/core/globals.cpp
    src/core/globals.hpp
    src/core/paths.cpp
    src/core/paths.hpp
    src/core/types.cpp
    src/core/types.hpp
    src/game_mode/exports/steam_api_flat.cpp
    src/game_mode/exports/steam_api_internal.cpp
    src/game_mode/exports/steam_api_unversioned.cpp
    src/game_mode/virtuals/isteamapps.cpp
    src/game_mode/virtuals/isteamclient.cpp
    src/game_mode/virtuals/isteaminventory.cpp
    src/game_mode/virtuals/isteamuser.cpp
    src/game_mode/virtuals/steam_api_virtuals.hpp
    src/smoke_api/config.cpp
    src/smoke_api/config.hpp
    src/smoke_api/smoke_api.cpp
    src/smoke_api/smoke_api.hpp
    src/steam_impl/steam_apps.cpp
    src/steam_impl/steam_apps.hpp
    src/steam_impl/steam_client.cpp
    src/steam_impl/steam_client.hpp
    src/steam_impl/steam_impl.cpp
    src/steam_impl/steam_impl.hpp
    src/steam_impl/steam_inventory.cpp
    src/steam_impl/steam_inventory.hpp
    src/steam_impl/steam_user.cpp
    src/steam_impl/steam_user.hpp
	src/steam_impl/steam_game_server.cpp
	src/steam_impl/steam_game_server.hpp
    src/main.cpp
    ${GENERATED_LINKER_EXPORTS}
)

# Include store_mode mode sources only in 32-bit builds
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(
        SMOKE_API_SOURCES ${SMOKE_API_SOURCES}
        src/store_mode/steamclient/client_app_manager.cpp
        src/store_mode/steamclient/client_apps.cpp
        src/store_mode/steamclient/client_inventory.cpp
        src/store_mode/steamclient/client_user.cpp
        src/store_mode/steamclient/client_utils.cpp
        src/store_mode/steamclient/steamclient.cpp
        src/store_mode/steamclient/steamclient.hpp
        src/store_mode/vstdlib/vstdlib.cpp
        src/store_mode/vstdlib/vstdlib.hpp
        src/store_mode/store.cpp
        src/store_mode/store.hpp
        src/store_mode/store_api.cpp
        src/store_mode/store_api.hpp
        src/store_mode/store_cache.cpp
        src/store_mode/store_cache.hpp
    )
endif ()

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES} ${VERSION_RESOURCE})

configure_output_name(${STEAMAPI_DLL})

configure_include_directories()

target_link_libraries(SmokeAPI PRIVATE KoalaBox)