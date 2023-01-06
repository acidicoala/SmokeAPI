#include <koalabox/koalabox.hpp>
#include <steam_functions/steam_functions.hpp>

namespace steam_user {
    using namespace koalabox;

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const std::function<EUserHasLicenseForAppResult()>& original_function
    );

}
