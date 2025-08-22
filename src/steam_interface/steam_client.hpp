#pragma once

#include "smoke_api/types.hpp"

namespace steam_client {
    void* GetGenericInterface(
        const std::string& function_name,
        const std::string& interface_version,
        const std::function<void*()>& original_function
    );
}
