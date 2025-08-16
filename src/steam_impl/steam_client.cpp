#include <steam_impl/steam_client.hpp>
#include <steam_impl/steam_impl.hpp>
#include <koalabox/logger.hpp>

namespace steam_client {

    void* GetGenericInterface(
        const String& function_name,
        const String& interface_version,
        const Function<void*()>& original_function
    ) {
        auto* const interface = original_function();

        LOG_DEBUG("{} -> '{}' @ {}", function_name, interface_version, interface);

        steam_impl::hook_virtuals(interface, interface_version);

        return interface;
    }

}
