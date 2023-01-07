#pragma once

#include <core/types.hpp>
#include <koalabox/core.hpp>

namespace steam_user {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appID,
        const Function<EUserHasLicenseForAppResult()>& original_function
    );

}
