#pragma once

#include <koalabox/core.hpp>

namespace paths {

    /**
     * @return An std::path instance representing the directory containing this DLL
     */
    Path get_self_path();
    Path get_config_path();
    Path get_cache_path();
    Path get_log_path();

}
