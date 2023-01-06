#include <koalabox/koalabox.hpp>
#include <steam_functions/steam_functions.hpp>

namespace steam_client {
    using namespace koalabox;

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const std::function<void*()>& original_function
    );

}
