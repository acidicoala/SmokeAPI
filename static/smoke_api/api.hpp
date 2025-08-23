#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api::api {
    std::optional<std::vector<DLC>> fetch_dlcs_from_github(AppId_t app_id);

    std::optional<std::vector<DLC>> fetch_dlcs_from_steam(AppId_t app_id);
}
