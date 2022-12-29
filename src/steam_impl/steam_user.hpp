#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

using namespace smoke_api;

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const std::function<EUserHasLicenseForAppResult()>& original_function
    );

}
