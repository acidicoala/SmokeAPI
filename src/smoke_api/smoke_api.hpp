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

    extern HMODULE self_module;

    extern HMODULE original_library;

    extern bool is_hook_mode;

    void init(HMODULE module_handle);

    void shutdown();

}
