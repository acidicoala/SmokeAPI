#pragma once

#include <functional>

namespace steam_client {
    void* GetGenericInterface(
        const std::string& function_name,
        const char* interface_version,
        const std::function<void*()>& original_function
    ) noexcept;
}
