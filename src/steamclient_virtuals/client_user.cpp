#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

using namespace smoke_api;

// TODO: Implement?
VIRTUAL(bool) IClientUser_IsSubscribedApp(PARAMS(AppId_t app_id)) { // NOLINT(misc-unused-parameters)
    return steam_apps::IsDlcUnlocked(__func__, 0, app_id);
}
