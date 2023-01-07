#pragma once
#include <koalabox/types.hpp>

namespace globals {

    extern HMODULE smokeapi_handle;
    extern HMODULE steamclient_module;
    extern HMODULE steamapi_module;
    extern HMODULE vstdlib_module;
    extern Map<String, uintptr_t> address_map;

}
