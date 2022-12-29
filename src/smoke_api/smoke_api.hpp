#pragma once

#include <koalabox/koalabox.hpp>
#include <koalabox/hook.hpp> // For macros

#include <nlohmann/json.hpp>

#define GET_ORIGINAL_FUNCTION(FUNC) \
    static const auto FUNC##_o = hook::get_original_function( \
        smoke_api::is_hook_mode, \
        smoke_api::original_library, \
        #FUNC, \
        FUNC \
    );

#define GET_ORIGINAL_VIRTUAL_FUNCTION(FUNC) \
    const auto FUNC##_o = hook::get_original_function( \
        true, \
        smoke_api::original_library, \
        #FUNC, \
        FUNC \
    );

namespace smoke_api {
    using namespace koalabox;

    struct KoalageddonConfig {
        String interface_interceptor_pattern = "55 8B EC 8B ?? ?? ?? ?? ?? 81 EC ?? ?? ?? ?? 53 FF 15";

        // Offset values are interpreted according to pointer arithmetic rules, i.e.
        // 1 unit offset represents 4 and 8 bytes in 32-bit and 64-bit architectures respectively.
        uint32_t callback_interceptor_address_offset = 1;
        uint32_t callback_address_offset = 20;
        uint32_t callback_data_offset = 0;
        uint32_t callback_name_offset = 4;

        // We do not use *_WITH_DEFAULT macro to ensure that overriding
        // the koalageddon config requires definition of all keys
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            KoalageddonConfig, // NOLINT(misc-const-correctness)
            interface_interceptor_pattern,
            callback_interceptor_address_offset,
            callback_address_offset,
            callback_data_offset,
            callback_name_offset
        )
    };

    struct Config {
        uint32_t $version = 2;
        bool logging = false;
        bool unlock_all = true;
        Set<uint32_t> override;
        Vector<uint32_t> dlc_ids;
        bool auto_inject_inventory = true;
        Vector<uint32_t> inventory_items;

        // Have to use general json type here since library doesn't support std::optional
        nlohmann::json koalageddon_config;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config, $version, // NOLINT(misc-const-correctness)
            logging,
            unlock_all,
            override,
            dlc_ids,
            auto_inject_inventory,
            inventory_items,
            koalageddon_config
        )
    };

    extern Config config;

    extern KoalageddonConfig koalageddon_config;

    extern HMODULE original_library;

    extern bool is_hook_mode;

    extern Path self_directory;

    void init(HMODULE self_module);

    void shutdown();

    bool should_unlock(uint32_t app_id);

}
