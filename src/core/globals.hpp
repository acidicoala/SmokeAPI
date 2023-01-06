#pragma once
#include <koalabox/koalabox.hpp>

namespace globals {
    using namespace koalabox;

    extern HMODULE smokeapi_handle;
    extern HMODULE steamclient_module;
    extern HMODULE steamapi_module;
    extern HMODULE vstdlib_module;
    extern Map<String, FunctionAddress> address_map;
}
