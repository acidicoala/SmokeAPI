#include <steam_impl/steam_client.hpp>

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const std::function<void*()>& original_function
    ) {
        logger->debug("{} -> Version: '{}'", function_name, interface_version);

        auto* const interface = original_function();

        logger->debug("{} -> Result: {}", function_name, fmt::ptr(interface));

        steam_functions::hook_virtuals(interface, interface_version);

        return interface;
    }

}
