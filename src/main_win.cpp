// ReSharper disable CppUnusedIncludeDirective
#include <koalabox/win.hpp>

#include "smoke_api/smoke_api.hpp"

// These headers will be populated at build time
#include "linker_exports_for_steam_api.h"
#include "linker_exports_for_windows_dlls.h"

DLL_MAIN(void* handle, const uint32_t reason, void*) {
    if(reason == DLL_PROCESS_ATTACH) {
        smoke_api::init(handle);
    } else if(reason == DLL_PROCESS_DETACH) {
        smoke_api::shutdown();
    }

    return TRUE;
}
