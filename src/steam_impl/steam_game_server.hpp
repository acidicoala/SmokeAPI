#pragma once

#include <core/types.hpp>

namespace steam_game_server {

    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const String& function_name,
        AppId_t appId,
        AppId_t dlcId,
        const Function<EUserHasLicenseForAppResult()>& original_function
    );

}
