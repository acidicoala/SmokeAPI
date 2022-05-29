#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_impl.hpp>

using namespace smoke_api;

VIRTUAL(bool) IClientAppManager_IsAppDlcInstalled(
    PARAMS( // NOLINT(misc-unused-parameters)
        AppId_t app_id,
        AppId_t dlc_id
    )
) {
    return steam_apps::IsDlcUnlocked(__func__, app_id, dlc_id);
}