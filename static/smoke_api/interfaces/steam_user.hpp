#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api::steam_user {
    EUserHasLicenseForAppResult UserHasLicenseForApp(
        const std::string& function_name,
        AppId_t appId,
        AppId_t dlcId,
        const std::function<EUserHasLicenseForAppResult()>& original_function
    ) noexcept;
}
