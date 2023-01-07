#include <steam_impl/steam_client.hpp>
#include <koalabox/logger.hpp>
#include <steam_functions/steam_functions.hpp>

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const Function<void*()>& original_function
    ) {
        auto* const interface = original_function();

        LOG_DEBUG("{} -> '{}' @ {}", function_name, interface_version, interface)

        steam_functions::hook_virtuals(interface, interface_version);

        return interface;
    }

}
