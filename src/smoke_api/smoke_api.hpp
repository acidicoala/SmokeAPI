#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api {
    void init(void* self_module_handle);
    void shutdown();

    AppId_t get_app_id();
}
