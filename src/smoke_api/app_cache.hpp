#pragma once

#include <core/types.hpp>

namespace smoke_api::app_cache {

    Vector<AppId_t> get_dlc_ids(AppId_t app_id);

    bool save_dlc_ids(AppId_t app_id, const Vector<AppId_t>& dlc_ids);

}
