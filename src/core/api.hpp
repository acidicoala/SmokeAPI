#pragma once

#include <core/types.hpp>

namespace api {

    std::optional<Vector<DLC>> fetch_dlcs_from_github(AppId_t app_id) noexcept;

    std::optional<Vector<DLC>> fetch_dlcs_from_steam(AppId_t app_id) noexcept;

}
