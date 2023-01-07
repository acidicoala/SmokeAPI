#include <steam_impl/steam_user.hpp>
#include <core/config.hpp>
#include <koalabox/logger.hpp>

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const Function<EUserHasLicenseForAppResult()>& original_function
    ) {
        const auto result = original_function();

        if (result == k_EUserHasLicenseResultNoAuth) {
            LOG_WARN("{} -> App ID: {}, Result: NoAuth", function_name, appID)
            return result;
        }

        const auto has_license = config::is_dlc_unlocked(
            0, appID, [&]() {
                return result == k_EUserHasLicenseResultHasLicense;
            }
        );

        LOG_INFO("{} -> App ID: {}, HasLicense: {}", function_name, appID, has_license)

        return has_license
               ? k_EUserHasLicenseResultHasLicense
               : k_EUserHasLicenseResultDoesNotHaveLicense;
    }

}
