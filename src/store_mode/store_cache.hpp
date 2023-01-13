#pragma once

#include <store_mode/store.hpp>

namespace store::store_cache {

    std::optional<StoreConfig> get_store_config();

    bool save_store_config(const StoreConfig& config);

}
