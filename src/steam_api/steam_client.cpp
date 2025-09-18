#include <koalabox/logger.hpp>

#include "steam_client.hpp"
#include "steam_api/steam_interfaces.hpp"

namespace steam_client {
    void* GetGenericInterface(
        const std::string& function_name,
        const char* interface_version, // This can be null
        const std::function<void*()>& original_function
    ) noexcept {
        try {
            auto* const interface = original_function();

            if(interface_version) {
                LOG_DEBUG("{} -> '{}' @ {}", function_name, interface_version, interface);

                steam_interfaces::hook_virtuals(interface, interface_version);
            }

            return interface;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: '{}' @ {}", function_name, interface_version, e.what());
            return nullptr;
        }
    }
}
