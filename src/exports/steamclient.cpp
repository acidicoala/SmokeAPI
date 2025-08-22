#include <koalabox/hook.hpp>

#include "exports/steamclient.hpp"
#include "smoke_api/types.hpp"
#include "steam_interface/steam_client.hpp"

C_DECL(void*) CreateInterface(const char* interface_string, int* out_result) {
    return steam_client::GetGenericInterface(__func__, interface_string, [&] {
        GET_ORIGINAL_HOOKED_FUNCTION(CreateInterface)

        return CreateInterface_o(interface_string, out_result);
    });
}
