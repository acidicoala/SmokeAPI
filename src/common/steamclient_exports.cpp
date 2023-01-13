#include <common/steamclient_exports.hpp>
#include <steam_impl/steam_client.hpp>

DLL_EXPORT(void*) CreateInterface(const char* interface_string, int* out_result) {
    return steam_client::GetGenericInterface(
        __func__, interface_string, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(CreateInterface)

            return CreateInterface_o(interface_string, out_result);
        }
    );
}
