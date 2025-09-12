#include <dlfcn.h>

#include "smoke_api/smoke_api.hpp"

extern "C" void __attribute__((constructor)) on_loaded() {
    // On linux we don't automatically get current module handle,
    // hence we get it manually
    Dl_info info;
    if(dladdr(reinterpret_cast<void*>(&on_loaded), &info) && info.dli_fname) {
        void* handle = dlopen(info.dli_fname, RTLD_NOW | RTLD_NOLOAD);
        smoke_api::init(handle);
    } else {
        OutputDebugString("Initialization error: failed to get own module handle.");
        DebugBreak();
    }
}

extern "C" void __attribute__((destructor)) on_unloaded() {
    smoke_api::shutdown();
}
