#include <core/globals.hpp>

namespace globals {
    // TODO: Refactor to koalabox?
    HMODULE smokeapi_handle = nullptr;
    HMODULE steamapi_module = nullptr;
    HMODULE vstdlib_module = nullptr;
    HMODULE steamclient_module = nullptr;
    Map<String, uintptr_t> address_map; // NOLINT(cert-err58-cpp)

}
