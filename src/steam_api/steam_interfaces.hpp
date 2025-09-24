#pragma once

#include <map>
#include <string>

#define SMK_FIND_INTERFACE_FUNC(INTERFACE_PTR, INTERFACE_NAME, FUNCTION_NAME) \
    reinterpret_cast<decltype(&INTERFACE_NAME##_##FUNCTION_NAME)>( \
        steam_interfaces::find_function(INTERFACE_PTR, #INTERFACE_NAME, #FUNCTION_NAME) \
    )

namespace steam_interfaces {
    constexpr auto STEAM_CLIENT = "SteamClient";

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

    void* find_function(
        const void* instance_ptr,
        const std::string& interface_name,
        const std::string& function_name
    );

    const std::map<std::string, std::string>& get_interface_name_to_version_map();
}
