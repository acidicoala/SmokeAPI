#pragma once

#include <koalabox/koalabox.hpp>
#include <core/macros.hpp>
#include <nlohmann/json.hpp>

namespace koalageddon {
    using namespace koalabox;

    struct KoalageddonConfig {
        String interface_interceptor_pattern = "55 8B EC 8B ?? ?? ?? ?? ?? 81 EC ?? ?? ?? ?? 53 FF 15";

        // Offset values are interpreted according to pointer arithmetic rules, i.e.
        // 1 unit offset represents 4 and 8 bytes in 32-bit and 64-bit architectures respectively.
        uint32_t callback_interceptor_address_offset = 1;
        uint32_t callback_address_offset = 20;
        uint32_t callback_data_offset = 0;
        uint32_t callback_name_offset = 4;
        uint32_t steamclient_interceptor_function_address_offset = 2;
        uint32_t steamclient_interceptor_function_address_offset_client_user = 5;
        uint32_t IClientAppManager_IsAppDlcInstalled_ordinal = 8;
        uint32_t IClientApps_GetDLCCount_ordinal = 8;
        uint32_t IClientApps_BGetDLCDataByIndex_ordinal = 9;
        uint32_t IClientInventory_GetResultStatus_ordinal = 0;
        uint32_t IClientInventory_GetResultItems_ordinal = 2;
        uint32_t IClientInventory_GetResultItemProperty_ordinal = 3;
        uint32_t IClientInventory_CheckResultSteamID_ordinal = 5;
        uint32_t IClientInventory_GetAllItems_ordinal = 8;
        uint32_t IClientInventory_GetItemsByID_ordinal = 9;
        uint32_t IClientInventory_SerializeResult_ordinal = 6;
        uint32_t IClientInventory_GetItemDefinitionIDs_ordinal = 19;
        uint32_t IClientUser_BIsSubscribedApp_ordinal = 191;

        // We do not use *_WITH_DEFAULT macro to ensure that overriding
        // the koalageddon config requires definition of all keys
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
            KoalageddonConfig, // NOLINT(misc-const-correctness)
            interface_interceptor_pattern,
            callback_interceptor_address_offset,
            callback_address_offset,
            callback_data_offset,
            callback_name_offset,
            steamclient_interceptor_function_address_offset,
            steamclient_interceptor_function_address_offset_client_user,
            IClientAppManager_IsAppDlcInstalled_ordinal,
            IClientApps_GetDLCCount_ordinal,
            IClientApps_BGetDLCDataByIndex_ordinal,
            IClientInventory_GetResultStatus_ordinal,
            IClientInventory_GetResultItems_ordinal,
            IClientInventory_GetResultItemProperty_ordinal,
            IClientInventory_CheckResultSteamID_ordinal,
            IClientInventory_GetAllItems_ordinal,
            IClientInventory_GetItemsByID_ordinal,
            IClientInventory_SerializeResult_ordinal,
            IClientInventory_GetItemDefinitionIDs_ordinal,
            IClientUser_BIsSubscribedApp_ordinal
        )
    };

    extern KoalageddonConfig config;

    void init();
}

typedef uint32_t HCoroutine;

DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, struct CoroutineData* data);
DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name);
