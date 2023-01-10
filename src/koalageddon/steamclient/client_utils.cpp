#include <koalageddon/steamclient/steamclient.hpp>

VIRTUAL(AppId_t) IClientUtils_GetAppID(PARAMS()) {
    GET_ORIGINAL_HOOKED_FUNCTION(IClientUtils_GetAppID)

    return IClientUtils_GetAppID_o(ARGS());
}
