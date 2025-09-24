#pragma once

#include "smoke_api/types.hpp"


// IMPORTANT: DLL_EXPORT is hardcoded in windows_exports_generator.cpp & linux_exports_generator.cpp,
// so any name changes here must be reflected there as well.
#define DLL_EXPORT(TYPE) extern "C" [[maybe_unused]] __declspec(dllexport) TYPE __cdecl

namespace smoke_api {
    void init(void* module_handle);
    void shutdown();

    AppId_t get_app_id();
}
