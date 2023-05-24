#include <steam_impl/steam_game_server.hpp>
#include <smoke_api/config.hpp>
#include <koalabox/logger.hpp>

namespace steam_game_server {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appId,
        AppId_t dlcId,
        const Function<EUserHasLicenseForAppResult()>& original_function
    ) {
        const auto result = original_function();

        if (result == k_EUserHasLicenseResultNoAuth) {
            LOG_WARN("{} -> App ID: {:>8}, Result: NoAuth", function_name, dlcId)
                return result;
        }

        const auto has_license = smoke_api::config::is_dlc_unlocked(
            appId, dlcId, [&]() {
                return result == k_EUserHasLicenseResultHasLicense;
            }
        );

        LOG_INFO("{} -> App ID: {:>8}, HasLicense: {}", function_name, dlcId, has_license)

            return has_license
            ? k_EUserHasLicenseResultHasLicense
            : k_EUserHasLicenseResultDoesNotHaveLicense;
    }

}
