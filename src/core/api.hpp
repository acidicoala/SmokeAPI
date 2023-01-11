#pragma once

#include <koalageddon/koalageddon.hpp>

namespace api {

    std::optional<Vector<DLC>> fetch_dlcs_from_github(AppId_t app_id) noexcept;

    std::optional<Vector<DLC>> fetch_dlcs_from_steam(AppId_t app_id) noexcept;

    std::optional<koalageddon::KoalageddonConfig> fetch_koalageddon_config() noexcept;

}
