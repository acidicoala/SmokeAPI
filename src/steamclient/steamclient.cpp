#include <koalabox/hook.hpp>

#include "smoke_api/steamclient/steamclient.hpp"

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/types.hpp"
#include "steam_api/steam_client.hpp"

/**
 * SmokeAPI implementation
 */
C_DECL(void*) CreateInterface(const char* interface_version, int* out_result) {
    return steam_client::GetGenericInterface(
        __func__,
        interface_version,
        [&] {
            static const auto CreateInterface$ = KB_HOOK_GET_HOOKED_FN(CreateInterface);
            return CreateInterface$(interface_version, out_result);
        }
    );
}
