cmake_minimum_required(VERSION 3.22)

project(SmokeAPI VERSION 1.0.0)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox EXCLUDE_FROM_ALL)

set_32_and_64(ORIGINAL_DLL steam_api)
set_32_and_64(STEAMCLIENT_DLL steamclient)
set_32_and_64(VSTDLIB_DLL vstdlib_s)

configure_version_resource("Free DLC for everyone ʕ ᵔᴥᵔʔ")

# Setup linker exports

set_32_and_64(DLL_SUFFIX steam_api win64/steam_api64)
file(GLOB DLL_INPUT "res/dll/*/sdk/redistributable_bin/${DLL_SUFFIX}.dll")

set(
    STEAM_API_EXPORTS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/steam_api_exports/steam_api_flat.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/steam_api_exports/steam_api_internal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/steam_api_exports/steam_api_unversioned.cpp"
)

configure_linker_exports(
    FORWARDED_DLL "${ORIGINAL_DLL}_o"
    INPUT_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/steam_api_exports"
    INPUT_DLLS "${DLL_INPUT}"
    DEP_SOURCES "${STEAM_API_EXPORTS}"
)

configure_build_config(extra_build_config)

set(
    SMOKE_API_SOURCES
    src/smoke_api/smoke_api.cpp
    src/smoke_api/smoke_api.hpp
    src/steam_api_exports/steam_api_flat.cpp
    src/steam_api_exports/steam_api_internal.cpp
    src/steam_api_exports/steam_api_unversioned.cpp
    src/steam_api_virtuals/isteamapps.cpp
    src/steam_api_virtuals/isteamclient.cpp
    src/steam_api_virtuals/isteaminventory.cpp
    src/steam_api_virtuals/isteamuser.cpp
    src/steam_impl/steam_apps.cpp
    src/steam_impl/steam_client.cpp
    src/steam_impl/steam_impl.hpp
    src/steam_impl/steam_inventory.cpp
    src/steam_impl/steam_user.cpp
    src/steam_functions/steam_functions.cpp
    src/steam_functions/steam_functions.hpp
    src/steam_types/steam_types.hpp
    src/steamclient_exports/steamclient.cpp
    src/vstdlib/vstdlib.cpp
    src/main.cpp
    ${GENERATED_LINKER_EXPORTS}
)

add_library(SmokeAPI SHARED ${SMOKE_API_SOURCES} ${VERSION_RESOURCE})

configure_output_name(${ORIGINAL_DLL})

configure_include_directories()

target_link_libraries(SmokeAPI PRIVATE KoalaBox)
