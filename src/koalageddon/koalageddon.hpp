#pragma once

#include <koalageddon/types.hpp>

namespace koalageddon {

    /// We need this interface in other IClient* functions in order to query original DLC status
    extern const void* client_app_manager_interface;

    extern KoalageddonConfig config;

    void init();

}
