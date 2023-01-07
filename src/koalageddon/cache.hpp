#pragma once

#include <koalageddon/types.hpp>

namespace koalageddon::cache {

    std::optional<KoalageddonConfig> get_koalageddon_config();

    bool save_koalageddon_config(const KoalageddonConfig& config);

}
