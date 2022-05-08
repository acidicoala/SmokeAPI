#include <smoke_api/smoke_api.hpp>

// This header will be populated at build time
#include <linker_exports.h>

EXTERN_C [[maybe_unused]] BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        smoke_api::init(module);
    } else if (reason == DLL_PROCESS_DETACH) {
        smoke_api::shutdown();
    }

    return TRUE;
}
