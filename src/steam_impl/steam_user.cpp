#include <steam_impl/steam_impl.hpp>
#include <smoke_api/smoke_api.hpp>

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const std::function<EUserHasLicenseForAppResult()>& original_function
    ) {
        const auto result = original_function();

        if (result == k_EUserHasLicenseResultNoAuth) {
            logger->warn("{} -> App ID: {}, Result: NoAuth", function_name, appID);
            return result;
        }

        const auto has_license = smoke_api::should_unlock(appID);

        logger->info("{} -> App ID: {}, HasLicense: {}", function_name, appID, has_license);

        return has_license
               ? k_EUserHasLicenseResultHasLicense
               : k_EUserHasLicenseResultDoesNotHaveLicense;
    }

}
