#include <build_config.h>
#include <smoke_api/smoke_api.hpp>
#include <koalageddon/koalageddon.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>
#include <steam_functions/steam_functions.hpp>

using namespace smoke_api;

/*
 * We need an interface=>function ordinal map in order to hook steam interface function
 * via virtual function pointer swap. We could construct it by exploiting these code chunks:
 *
 * 8B01         | mov eax, dword ptr ds:[ecx]
 * 68 ????????  | push steamclient.function_name
 * 68 ????????  | push steamclient.interface_name
 *
 * Step 1: Find all addresses that begin with pattern
 *         8B 01 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 10
 * Step 2: Extract function and interface name pointers by adding 3 and 8 respectively
 * Step 3: Starting from the found address, and until the function epilogue, scan for either instructions:
 *
 *         (1) FF50 ?? | call dword ptr ds:[eax+0x??]
 *         or
 *         (2) 8B40 ?? | mov eax, dword ptr ds:[eax+??]
 *             FFD0    | call eax
 *
 *         In the case (1), the offset is encoded in the found call instruction.
 *         In the case (2), the offset is encoded in the instruction preceding the call.
 *
 *         ROADBLOCK: There is actually a case (3) which calls a local variable (ebp-??),
 *         which itself is constructed over multiple instruction calls, making it non-deterministic.
 *         Until this roadblock is resolved, automatic detection of ordinals remains non-viable.
 */
[[maybe_unused]] Map<String, uint32_t> construct_interface_function_ordinal_map() {
    auto* const steamclient_handle = win_util::get_module_handle_or_throw(STEAMCLIENT_DLL);
    const auto steamclient_module_info = win_util::get_module_info_or_throw(steamclient_handle);

    auto* byte_pointer = (uint8_t*) steamclient_module_info.lpBaseOfDll;
    while (byte_pointer) {
        // Search until the end of DLL section
        auto rva = (DWORD) byte_pointer - (DWORD) steamclient_module_info.lpBaseOfDll;

        // This pattern needs to be parameterized if this method ever gets implemented.
        const String interface_function_chunk_pattern = "8B 01 68 ?? ?? ?? ?? 68";

        byte_pointer = reinterpret_cast<uint8_t*>(
            patcher::find_pattern_address(
                byte_pointer,
                steamclient_module_info.SizeOfImage - rva,
                "interface=>function chunk",
                interface_function_chunk_pattern,
                false
            )
        );

        // const auto* interface_name = *reinterpret_cast<char**>(byte_pointer + 3);
        // const auto* function_name = *reinterpret_cast<char**>(byte_pointer + 8);

        byte_pointer += interface_function_chunk_pattern.size();
        byte_pointer += interface_function_chunk_pattern.size();

        static const auto is_epilogue = [](const uint8_t* instruction_byte) {
            const uint8_t epilogue[]{
                0x8B, 0xE5, //  mov esp,ebp
                0x5D,       //  pop ebp
                0xC3,       //  ret
            };

            for (auto i = 0; i < sizeof(epilogue); i++) {
                if (epilogue[i] != instruction_byte[i]) {
                    return false;
                }
            }

            return true;
        };

        static const auto is_call_dword = [](const uint8_t* instruction_byte) {
            return instruction_byte[0] == 0xFF && instruction_byte[1] == 0x50;
        };

        // static const auto is_call_eax = [](const uint8_t* instruction_byte) {
        //     return instruction_byte[0] == 0x8B && instruction_byte[1] == 0x40 &&
        //            instruction_byte[3] == 0xFF && instruction_byte[4] == 0xD0;
        // };

        while (!is_epilogue(byte_pointer)) {
            if (is_call_dword(byte_pointer)) {
                // Find a way to determine offset
            }

            byte_pointer++;
        }
    }

    return {};
}

#define DEFINE_INTERFACE(INTERFACE) constexpr auto INTERFACE = #INTERFACE;

DEFINE_INTERFACE(IClientAppManager)
DEFINE_INTERFACE(IClientApps)
DEFINE_INTERFACE(IClientInventory)
DEFINE_INTERFACE(IClientUser)

DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name) {
    try {
        // logger->trace("Intercepted interface function: '{}::{}'", interface_name, function_name);

        static Set<String> hooked_interfaces;

        if (hooked_interfaces.size() < 4) {
            const void***** current_ebp;
            __asm mov current_ebp, ebp

            auto* const parent_ebp = *current_ebp;

            const auto* interface_address = parent_ebp[
                util::strings_are_equal(interface_name, IClientUser)
                ? koalageddon::config.steamclient_interceptor_function_address_offset_client_user
                : koalageddon::config.steamclient_interceptor_function_address_offset
            ];

            const auto hook_if_needed = [&](const String& name, const std::function<void()>& block) {

                const auto is_target_interface = interface_name == name;
                const auto is_hooked = hooked_interfaces.contains(name);
                const auto is_valid_address = interface_address != nullptr;

                if (is_target_interface && !is_hooked && is_valid_address) {
                    block();
                    hooked_interfaces.insert(name);
                }
            };

#define HOOK_INTERFACE(FUNC) hook::swap_virtual_func_or_throw( \
    interface_address,                                         \
    #FUNC,                                                     \
    koalageddon::config.FUNC##_ordinal,                         \
    (FunctionAddress) FUNC                                     \
);

            hook_if_needed(IClientAppManager, [&]() {
                HOOK_INTERFACE(IClientAppManager_IsAppDlcInstalled)
                // TODO: Investigate IClientAppManager::BIsDlcEnabled
                // TODO: Investigate IClientAppManager::GetDlcSizes
                // TODO: Investigate IClientAppManager::GetInstalledApps
            });

            hook_if_needed(IClientApps, [&]() {
                HOOK_INTERFACE(IClientApps_GetDLCCount)
                HOOK_INTERFACE(IClientApps_BGetDLCDataByIndex)
                // TODO: Investigate IClientApps::GetAppKVRaw
                // TODO: Investigate IClientApps::GetAppDataSection
                // TODO: Investigate IClientApps::GetAppData
            });

            hook_if_needed(IClientInventory, [&]() {
                HOOK_INTERFACE(IClientInventory_GetResultStatus)
                HOOK_INTERFACE(IClientInventory_GetResultItems)
                HOOK_INTERFACE(IClientInventory_GetResultItemProperty)
                HOOK_INTERFACE(IClientInventory_CheckResultSteamID)
                HOOK_INTERFACE(IClientInventory_GetAllItems)
                HOOK_INTERFACE(IClientInventory_GetItemsByID)
                HOOK_INTERFACE(IClientInventory_SerializeResult)
                HOOK_INTERFACE(IClientInventory_GetItemDefinitionIDs)
            });

            hook_if_needed(IClientUser, [&]() {
                // NOTE: parent_ebp[function_offset] will always be 0 for IClientUser interface.
                // Probably because it actually represents a handle with 0 being the default user.
                // However, the real interface is still accessible at a neighboring offset.
                HOOK_INTERFACE(IClientUser_BIsSubscribedApp)
                // TODO: Investigate IClientUser::GetConfigString
                // TODO: Investigate IClientUser::GetConfigStoreKeyName
                // TODO: Investigate IClientUser::GetSubscribedApps
                // TODO: Investigate IClientUser::GetUserConfigFolder
                // TODO: Investigate IClientUser::GetAppIDForGameID
            });

            // TODO: Investigate IClientDeviceAuth::GetSharedLibraryLockedBy?
        }

        GET_ORIGINAL_FUNCTION(SteamClient_Interface_Interceptor)
        SteamClient_Interface_Interceptor_o(interface_name, function_name);
    } catch (const Exception& ex) {
        logger->error("{} -> Error: {}", __func__, ex.what());
    }
}
