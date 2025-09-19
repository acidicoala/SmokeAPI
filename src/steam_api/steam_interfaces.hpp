#pragma once

#include <string>

namespace steam_interfaces {
    void hook_virtuals(const void* interface_ptr, const std::string& version_string);

    /**
     * A fallback mechanism used when SteamAPI has already been initialized.
     * It will hook the SteamClient interface and hook its interface accessors.
     * This allows us to hook interfaces that are no longer being created,
     * such as in the case of late injection.
     */
    void hook_steamclient_interface(
        void* steamclient_handle,
        const std::string& steam_client_interface_version
    ) noexcept;
}
