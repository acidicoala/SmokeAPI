#include "smoke_api/steamclient/steamclient.hpp"
#include "smoke_api.hpp"
#include "smoke_api/types.hpp"

#include "steam_api/steam_client.hpp"

/**
 * SmokeAPI implementation
 */
C_DECL(void*) CreateInterface(const char* interface_version, int* out_result) {
    return steam_client::GetGenericInterface(
        __func__,
        interface_version,
        HOOKED_CALL_CLOSURE(CreateInterface, interface_version, out_result)
    );
}
