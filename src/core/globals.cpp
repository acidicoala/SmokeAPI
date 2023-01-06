#include <core/globals.hpp>

namespace globals {
    HMODULE smokeapi_handle = nullptr;
    HMODULE steamapi_module = nullptr;
    HMODULE vstdlib_module = nullptr;
    HMODULE steamclient_module = nullptr;
    Map<String, FunctionAddress> address_map;
}
