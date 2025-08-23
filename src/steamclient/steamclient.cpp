#include <koalabox/hook.hpp>

#include "steamclient.hpp"

#include "smoke_api.hpp"
#include "smoke_api/types.hpp"
#include "steam_api/steam_client.hpp"

C_DECL(void*) CreateInterface(const char* interface_string, int* out_result) {
    return steam_client::GetGenericInterface(
        __func__,
        interface_string,
        HOOKED_CALL_CLOSURE(CreateInterface, interface_string, out_result)
    );
}
