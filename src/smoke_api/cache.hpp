#pragma once

#include "types.hpp"

namespace smoke_api::cache {

    std::vector<DLC> get_dlcs(AppId_t app_id) noexcept;

    bool save_dlcs(AppId_t app_id, const std::vector<DLC>& dlcs) noexcept;

}
