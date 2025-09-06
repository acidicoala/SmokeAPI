#include <mutex>

#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>

#include "smoke_api/steamclient/steamclient.hpp"
#include "smoke_api/types.hpp"

#include "steam_api/steam_client.hpp"

/**
 * SmokeAPI implementation
 */
C_DECL(void*) CreateInterface(const char* interface_version, create_interface_result* out_result) {
    // Mutex here helps us detect unintended recursion early on by throwing an exception.
    static std::mutex section;
    const std::lock_guard lock(section);

    return steam_client::GetGenericInterface(
        __func__,
        interface_version,
        [&] {
            static const auto CreateInterface$ = KB_HOOK_GET_HOOKED_FN(CreateInterface);
            return CreateInterface$(interface_version, out_result);
        }
    );
}
