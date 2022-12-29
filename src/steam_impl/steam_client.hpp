#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

using namespace smoke_api;

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const std::function<void*()>& original_function
    );

}
