#include <store_mode/steamclient/steamclient.hpp>
#include <steam_impl/steam_apps.hpp>

VIRTUAL(bool) IClientUser_BIsSubscribedApp(PARAMS(AppId_t dlc_id)) {
    try {
        const auto* utils_interface = store::steamclient::interface_name_to_address_map["IClientUtils"];

        const auto app_id = utils_interface ? IClientUtils_GetAppID(utils_interface, EDX) : 0;

        return steam_apps::IsDlcUnlocked(__func__, app_id, dlc_id, [&]() {
            GET_ORIGINAL_HOOKED_FUNCTION(IClientUser_BIsSubscribedApp)

            return IClientUser_BIsSubscribedApp_o(ARGS(dlc_id));
        });
    } catch (const Exception& e) {
        LOG_ERROR("{} -> Error: {}", __func__, e.what())
        return false;
    }
}
