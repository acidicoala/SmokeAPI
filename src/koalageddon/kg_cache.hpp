#pragma once

#include <koalageddon/koalageddon.hpp>

namespace koalageddon::kg_cache {

    std::optional<KoalageddonConfig> get_koalageddon_config();

    bool save_koalageddon_config(const KoalageddonConfig& config);

}
