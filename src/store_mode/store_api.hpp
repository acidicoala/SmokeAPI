#pragma once

#include <core/types.hpp>
#include <store_mode/store.hpp>

namespace store::api {

    std::optional<store::StoreConfig> fetch_store_config() noexcept;

}