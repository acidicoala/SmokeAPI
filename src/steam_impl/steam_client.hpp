#pragma once

#include <koalabox/core.hpp>

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const Function<void*()>& original_function
    );

}
