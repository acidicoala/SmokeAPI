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
    static const auto FUNC##_o = hook::get_original_function( \
        true, \
        smoke_api::original_library, \
        #FUNC, \
        FUNC \
    );

namespace smoke_api {
    using namespace koalabox;

    struct Config {
        uint32_t $version = 1;
        bool logging = false;
        bool hook_steamclient = true;
        bool unlock_all = true;
        Set<uint32_t> override;
        Vector<uint32_t> dlc_ids;
        bool auto_inject_inventory = true;
        Vector<uint32_t> inventory_items;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config, $version,
            logging,
            unlock_all,
            hook_steamclient,
            override,
            dlc_ids,
            auto_inject_inventory,
            inventory_items
        )
    };

    extern Config config;

    extern HMODULE original_library;

    extern bool is_hook_mode;

    extern Path self_directory;

    void init(HMODULE self_module);

    void shutdown();

    bool should_unlock(uint32_t app_id);

}
