#pragma once

#include "smoke_api/types.hpp"

namespace steam_interface {
    AppId_t get_app_id_or_throw();
    AppId_t get_app_id();

    void hook_virtuals(void* interface, const std::string& version_string);
}
