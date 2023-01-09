#pragma once

#include <core/types.hpp>

namespace steam_impl {

    void hook_virtuals(void* interface, const String& version_string);

    uint32_t get_app_id_or_throw();

}
