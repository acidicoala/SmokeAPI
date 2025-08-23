#include "smoke_api.hpp"

// This header will be populated at build time
#include "linker_exports_for_steam_api.h"
#include "linker_exports_for_version.h"

EXTERN_C [[maybe_unused]] BOOL WINAPI
DllMain(const HMODULE module_handle, const DWORD reason, LPVOID) {
    if(reason == DLL_PROCESS_ATTACH) {
        smoke_api::init(module_handle);
    } else if(reason == DLL_PROCESS_DETACH) {
        smoke_api::shutdown();
    }

    return TRUE;
}
