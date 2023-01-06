#include <build_config.h>
#include <koalageddon/koalageddon.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>
#include <steam_functions/steam_functions.hpp>
#include <Zydis/Zydis.h>
#include <Zydis/DecoderTypes.h>

using namespace koalabox;

#define DEMUX_DECL(INTERFACE)                                                               \
    constexpr auto INTERFACE = #INTERFACE;                                                  \
    DLL_EXPORT(void) INTERFACE##_Demux(const void*, const void*, const void*, const void*); \

DEMUX_DECL(IClientAppManager)
DEMUX_DECL(IClientApps)
DEMUX_DECL(IClientInventory)
DEMUX_DECL(IClientUser)

DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name);

namespace koalageddon {
    // Maps interface name to interface pointer
    Map<String, const void*> interface_name_pointer_map{}; // NOLINT(cert-err58-cpp)
    std::mutex map_mutex;

    Set<String> hooked_interfaces; // NOLINT(cert-err58-cpp)

    ZydisDecoder decoder = {};

    constexpr auto INTERFACE_TARGET_COUNT = 4;

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

    FunctionAddress get_absolute_address(ZydisDecodedInstruction instruction, FunctionAddress address) {
        const auto op = instruction.operands[0];

        if (op.imm.is_relative) {
            ZyanU64 absolute_address;
            ZydisCalcAbsoluteAddress(&instruction, &op, address, &absolute_address);

            return absolute_address;
        }

        return (FunctionAddress) op.imm.value.u;
    }

    const char* find_interface_name(FunctionAddress demux_address) {
        auto* instruction_pointer = (uint8_t*) demux_address;
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, instruction_pointer, 1024, &instruction))) {
            if (instruction.mnemonic == ZYDIS_MNEMONIC_PUSH) {
                const auto op = instruction.operands[0];

                if (
                    op.type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
                    op.visibility == ZYDIS_OPERAND_VISIBILITY_EXPLICIT &&
                    op.encoding == ZYDIS_OPERAND_ENCODING_SIMM16_32_32
                    ) {
                    const auto* name_address = reinterpret_cast<char*>(op.imm.value.u);
                    const auto is_valid = util::is_valid_pointer(name_address);
                    if (is_valid && String(name_address).starts_with("IClient")) {
                        return name_address;
                    }
                }
            }
            instruction_pointer += instruction.length;
        }

        return nullptr;
    }

    void init_steamclient_hooks() {
        ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);

        const HMODULE module_handle = win_util::get_module_handle_or_throw(STEAMCLIENT_DLL);
        const auto module_info = win_util::get_module_info_or_throw(module_handle);

        const auto start_address = reinterpret_cast<FunctionAddress>(module_info.lpBaseOfDll);
        auto* terminal_address = (uint8_t*) (start_address + module_info.SizeOfImage);

        // TODO: There may actually be a way to find this address from "first principles"
        // SteamClient.Steam_CreateSteamPipe begins with a call to fun_alpha()
        // fun_alpha() calls fun_beta()
        // ordinal 18
        const auto* interface_demux_address = patcher::find_pattern_address(
            module_info,
            "interface demux",
            config.steamclient_interface_demux_pattern
        );

        // Then iterate over each function demux call

        auto* instruction_pointer = (uint8_t*) interface_demux_address;
        ZydisDecodedInstruction previous_instruction{};
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, instruction_pointer, 10, &instruction))) {
            if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP && previous_instruction.mnemonic == ZYDIS_MNEMONIC_CALL) {

                // For every such call, extract a function demux address
                const auto call_demux_address = (FunctionAddress) (
                    instruction_pointer - previous_instruction.length
                );
                const auto function_demux_address = get_absolute_address(
                    previous_instruction, call_demux_address
                );

                if (function_demux_address == 0) {
                    logger->warn("Failed to extract absolute address of call at {}", (void*) call_demux_address);
                } else {
                    // Then use this address to extract the interface name
                    const char* interface_name_address = find_interface_name(function_demux_address);

                    if (interface_name_address == nullptr) {
                        logger->warn(
                            "Failed to extract interface name address of function demux at {}",
                            (void*) function_demux_address
                        );
                    } else {
                        const String interface_name((char*) interface_name_address);

                        logger->debug("Detected interface: '{}'", interface_name);

                        // Finally, hook the demux functions of interest

                        if (IClientAppManager == interface_name) {
                            DETOUR_ADDRESS(IClientAppManager_Demux, function_demux_address)
                        } else if (IClientApps == interface_name) {
                            DETOUR_ADDRESS(IClientApps_Demux, function_demux_address)
                        } else if (IClientInventory == interface_name) {
                            DETOUR_ADDRESS(IClientInventory_Demux, function_demux_address)
                        } else if (IClientUser == interface_name) {
                            DETOUR_ADDRESS(IClientUser_Demux, function_demux_address)
                        }

                        // Update the terminal address to limit the search scope only to relevant portion of the code
                        auto* function_epilogue = (uint8_t*) get_absolute_address(
                            instruction, (FunctionAddress) instruction_pointer
                        );

                        if (function_epilogue == nullptr) {
                            logger->warn(
                                "Failed to extract absolute address of jmp at {}",
                                (void*) instruction_pointer
                            );
                        } else {
                            terminal_address = function_epilogue;
                        }
                    }
                }
            }

            previous_instruction = instruction;
            instruction_pointer += instruction.length;
            if (instruction_pointer >= terminal_address) {
                break;
            }
        }

        auto interface_interceptor_address = (FunctionAddress) patcher::find_pattern_address(
            module_info,
            "SteamClient_Interface_Interceptor",
            config.steamclient_interface_interceptor_pattern
        );

        if (interface_interceptor_address) {
            DETOUR_ADDRESS(SteamClient_Interface_Interceptor, interface_interceptor_address)
        }
    }

}

/**
 * This function intercepts interface name and function names, which we need to determine which functions to hook.
 * Unfortunately we can't reliably get interface pointer in this function, hence we need to hook corresponding
 * parent demux functions which will contain the interface pointer as the first parameter.
 */
DLL_EXPORT(void) SteamClient_Interface_Interceptor(const char* interface_name, const char* function_name) {
    try {
        const std::lock_guard<std::mutex> guard(koalageddon::map_mutex);

        const auto needs_hooking = koalageddon::hooked_interfaces.size() < koalageddon::INTERFACE_TARGET_COUNT;
        const auto has_interface_pointer = koalageddon::interface_name_pointer_map.contains(interface_name);

        // logger->trace(
        //     "Intercepted interface function: '{}::{}'",
        //     interface_name, function_name
        // );

        if (needs_hooking && has_interface_pointer) {
            const auto* interface_address = koalageddon::interface_name_pointer_map[interface_name];

            const auto hook_if_needed = [&](const String& name, const std::function<void()>& block) {
                const auto is_target_interface = interface_name == name;
                const auto is_hooked = koalageddon::hooked_interfaces.contains(name);
                const auto is_valid_address = interface_address != nullptr;

                if (is_target_interface && !is_hooked && is_valid_address) {
                    block();
                    koalageddon::hooked_interfaces.insert(name);
                }
            };

#define HOOK_INTERFACE(FUNC) hook::swap_virtual_func_or_throw( \
    globals::address_map,                                      \
    (void*) interface_address,                                 \
    #FUNC,                                                     \
    koalageddon::config.FUNC##_ordinal,                        \
    (FunctionAddress) FUNC                                     \
);

            hook_if_needed(IClientAppManager, [&]() {
                HOOK_INTERFACE(IClientAppManager_IsAppDlcInstalled)
            });

            hook_if_needed(IClientApps, [&]() {
                HOOK_INTERFACE(IClientApps_GetDLCCount)
                HOOK_INTERFACE(IClientApps_BGetDLCDataByIndex)
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
                HOOK_INTERFACE(IClientUser_BIsSubscribedApp)
            });
        }

        GET_ORIGINAL_VIRTUAL_FUNCTION(SteamClient_Interface_Interceptor)
        SteamClient_Interface_Interceptor_o(interface_name, function_name);
    } catch (const Exception& ex) {
        logger->error("{} -> Error: {}", __func__, ex.what());
    }
}


/**
 * This macro will generate a definition of a demux function,
 * which will cache the interface pointer in the local map.
 */
#define DEMUX_IMPL(INTERFACE)                                               \
DLL_EXPORT(void) INTERFACE##_Demux(                                         \
    const void* arg1,                                                       \
    const void* arg2,                                                       \
    const void* arg3,                                                       \
    const void* arg4                                                        \
) {                                                                         \
    if(!koalageddon::hooked_interfaces.contains(INTERFACE)){                \
        const std::lock_guard<std::mutex> guard(koalageddon::map_mutex);    \
        koalageddon::interface_name_pointer_map[INTERFACE] = arg1;          \
    }                                                                       \
    GET_ORIGINAL_VIRTUAL_FUNCTION(INTERFACE##_Demux)                                \
    INTERFACE##_Demux_o(arg1, arg2, arg3, arg4);                            \
}

DEMUX_IMPL(IClientAppManager)
DEMUX_IMPL(IClientApps)
DEMUX_IMPL(IClientInventory)
DEMUX_IMPL(IClientUser)
