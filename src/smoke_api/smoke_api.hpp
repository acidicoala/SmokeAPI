#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api {
    void init(void* self_module_handle);

    /**
     * Post-initialization procedures that must be done after the module is finished loading.
     * Reason being that on Windows we should not start new threads while being in DllMain callback,
     * otherwise we would run into deadlocks/race-conditions/undefined behavior.
     */
    void post_init();

    void shutdown();

    AppId_t get_app_id();
}
