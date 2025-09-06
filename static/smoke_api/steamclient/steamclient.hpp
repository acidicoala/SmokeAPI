#pragma once

#include "smoke_api/types.hpp"

enum class create_interface_result {
    Success = 0,
    Error   = 1,
};

/**
 * @param interface_version Example: STEAMAPPS_INTERFACE_VERSION008
 * @param out_result Pointer to the result enum that will be written to. Can be nullptr.
 */
C_DECL(void*) CreateInterface(const char* interface_version, create_interface_result* out_result);
