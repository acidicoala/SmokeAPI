#include <core/steam_types.hpp>
#include <koalabox/types.hpp>

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const Function<EUserHasLicenseForAppResult()>& original_function
    );

}
