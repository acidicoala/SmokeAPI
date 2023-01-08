#pragma once

#include <koalabox/core.hpp>
#include <koalabox/json.hpp>

// Offset values are interpreted according to pointer arithmetic rules, i.e.
// 1 unit offset represents 4 and 8 bytes in 32-bit and 64-bit architectures respectively.
struct KoalageddonConfig {
    uint32_t client_engine_steam_client_internal_ordinal = 12;
    uint32_t steam_client_internal_interface_selector_ordinal = 18;
    uint32_t vstdlib_callback_address_offset = 20;
    uint32_t vstdlib_callback_data_offset = 0;
    uint32_t vstdlib_callback_interceptor_address_offset = 1;
    uint32_t vstdlib_callback_name_offset = 4;

    // We do not use *_WITH_DEFAULT macro to ensure that overriding
    // the koalageddon config requires definition of all keys
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        KoalageddonConfig, // NOLINT(misc-const-correctness)
        client_engine_steam_client_internal_ordinal,
        steam_client_internal_interface_selector_ordinal,
        vstdlib_callback_address_offset,
        vstdlib_callback_data_offset,
        vstdlib_callback_interceptor_address_offset,
        vstdlib_callback_name_offset
    )
};
