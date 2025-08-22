#include <koalabox/logger.hpp>

#include "steam_interface/steam_client.hpp"
#include "steam_interface/steam_interface.hpp"

namespace steam_client {

    void* GetGenericInterface(
        const std::string& function_name,
        const std::string& interface_version,
        const std::function<void*()>& original_function
    ) {
        auto* const interface = original_function();

        LOG_DEBUG("{} -> '{}' @ {}", function_name, interface_version, interface);

        steam_interface::hook_virtuals(interface, interface_version);

        return interface;
    }

}
