#pragma once

#include <koalabox/koalabox.hpp>
#include <core/macros.hpp>
#include <nlohmann/json.hpp>

namespace koalageddon {
    using namespace koalabox;

    // Offset values are interpreted according to pointer arithmetic rules, i.e.
    // 1 unit offset represents 4 and 8 bytes in 32-bit and 64-bit architectures respectively.
    struct KoalageddonConfig {
        uint32_t IClientAppManager_IsAppDlcInstalled_ordinal = 8;
        uint32_t IClientApps_BGetDLCDataByIndex_ordinal = 9;
        uint32_t IClientApps_GetDLCCount_ordinal = 8;
        uint32_t IClientInventory_CheckResultSteamID_ordinal = 5;
        uint32_t IClientInventory_GetAllItems_ordinal = 8;
        uint32_t IClientInventory_GetItemDefinitionIDs_ordinal = 19;
        uint32_t IClientInventory_GetItemsByID_ordinal = 9;
        uint32_t IClientInventory_GetResultItemProperty_ordinal = 3;
        uint32_t IClientInventory_GetResultItems_ordinal = 2;
        uint32_t IClientInventory_GetResultStatus_ordinal = 0;
        uint32_t IClientInventory_SerializeResult_ordinal = 6;
        uint32_t IClientUser_BIsSubscribedApp_ordinal = 191;

        uint32_t client_engine_steam_client_internal_ordinal = 12;
        uint32_t steam_client_internal_interface_selector_ordinal = 18;

        uint32_t vstdlib_callback_address_offset = 20;
        uint32_t vstdlib_callback_data_offset = 0;
        uint32_t vstdlib_callback_interceptor_address_offset = 1;
        uint32_t vstdlib_callback_name_offset = 4;

        // We do not use *_WITH_DEFAULT macro to ensure that overriding
        // the koalageddon config requires definition of all keys
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            KoalageddonConfig, // NOLINT(misc-const-correctness)
            IClientAppManager_IsAppDlcInstalled_ordinal,
            IClientApps_BGetDLCDataByIndex_ordinal,
            IClientApps_GetDLCCount_ordinal,
            IClientInventory_CheckResultSteamID_ordinal,
            IClientInventory_GetAllItems_ordinal,
            IClientInventory_GetItemDefinitionIDs_ordinal,
            IClientInventory_GetItemsByID_ordinal,
            IClientInventory_GetResultItemProperty_ordinal,
            IClientInventory_GetResultItems_ordinal,
            IClientInventory_GetResultStatus_ordinal,
            IClientInventory_SerializeResult_ordinal,
            IClientUser_BIsSubscribedApp_ordinal,

            client_engine_steam_client_internal_ordinal,
            steam_client_internal_interface_selector_ordinal,

            vstdlib_callback_address_offset,
            vstdlib_callback_data_offset,
            vstdlib_callback_interceptor_address_offset,
            vstdlib_callback_name_offset
        )
    };

    extern KoalageddonConfig config;

    void init();

    void init_steamclient_hooks(const void* interface_selector_address);
    void init_steamclient_hooks2();
}
