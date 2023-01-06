#include <build_config.h>
#include <koalageddon/koalageddon.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>
#include <steam_functions/steam_functions.hpp>
#include <Zydis/Zydis.h>
#include <Zydis/DecoderTypes.h>

namespace koalageddon {
    using namespace koalabox;

    ZydisDecoder decoder = {};

#define HOOK_FUNCTION(FUNC) hook::swap_virtual_func_or_throw(  \
    globals::address_map,                                       \
    interface,                                                  \
    #FUNC,                                                      \
    koalageddon::config.FUNC##_ordinal,                         \
    (FunctionAddress) FUNC                                      \
);

    DLL_EXPORT(void) IClientAppManager_Selector(
        const void* interface,
        const void* arg2,
        const void* arg3,
        const void* arg4
    ) {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            HOOK_FUNCTION(IClientAppManager_IsAppDlcInstalled)
        });

        GET_ORIGINAL_HOOKED_FUNCTION(IClientAppManager_Selector)
        IClientAppManager_Selector_o(interface, arg2, arg3, arg4);
    }

    DLL_EXPORT(void) IClientApps_Selector(
        const void* interface,
        const void* arg2,
        const void* arg3,
        const void* arg4
    ) {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            HOOK_FUNCTION(IClientApps_GetDLCCount)
            HOOK_FUNCTION(IClientApps_BGetDLCDataByIndex)
        });

        GET_ORIGINAL_HOOKED_FUNCTION(IClientApps_Selector)
        IClientApps_Selector_o(interface, arg2, arg3, arg4);
    }

    DLL_EXPORT(void) IClientInventory_Selector(
        const void* interface,
        const void* arg2,
        const void* arg3,
        const void* arg4
    ) {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            HOOK_FUNCTION(IClientInventory_GetResultStatus)
            HOOK_FUNCTION(IClientInventory_GetResultItems)
            HOOK_FUNCTION(IClientInventory_GetResultItemProperty)
            HOOK_FUNCTION(IClientInventory_CheckResultSteamID)
            HOOK_FUNCTION(IClientInventory_GetAllItems)
            HOOK_FUNCTION(IClientInventory_GetItemsByID)
            HOOK_FUNCTION(IClientInventory_SerializeResult)
            HOOK_FUNCTION(IClientInventory_GetItemDefinitionIDs)
        });

        GET_ORIGINAL_HOOKED_FUNCTION(IClientInventory_Selector)
        IClientInventory_Selector_o(interface, arg2, arg3, arg4);
    }

    DLL_EXPORT(void) IClientUser_Selector(
        const void* interface,
        const void* arg2,
        const void* arg3,
        const void* arg4
    ) {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            HOOK_FUNCTION(IClientUser_BIsSubscribedApp)
        });

        GET_ORIGINAL_HOOKED_FUNCTION(IClientUser_Selector)
        IClientUser_Selector_o(interface, arg2, arg3, arg4);
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

    const char* find_interface_name(FunctionAddress selector_address) {
        auto* instruction_pointer = (uint8_t*) selector_address;
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

    void init_steamclient_hooks(const void* interface_selector_address) {
        ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);

        const HMODULE module_handle = win_util::get_module_handle_or_throw(STEAMCLIENT_DLL);
        const auto module_info = win_util::get_module_info_or_throw(module_handle);

        const auto start_address = reinterpret_cast<FunctionAddress>(module_info.lpBaseOfDll);
        auto* terminal_address = (uint8_t*) (start_address + module_info.SizeOfImage);

        // Then iterate over each function selector call

        auto* instruction_pointer = (uint8_t*) interface_selector_address;
        ZydisDecodedInstruction previous_instruction{};
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, instruction_pointer, 10, &instruction))) {
            if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP && previous_instruction.mnemonic == ZYDIS_MNEMONIC_CALL) {

                // For every such call, extract a function selector address
                const auto call_selector_address = (FunctionAddress) (
                    instruction_pointer - previous_instruction.length
                );
                const auto function_selector_address = get_absolute_address(
                    previous_instruction, call_selector_address
                );

                if (function_selector_address == 0) {
                    logger->warn("Failed to extract absolute address of call at {}", (void*) call_selector_address);
                } else {
                    // Then use this address to extract the interface name
                    const char* interface_name_address = find_interface_name(function_selector_address);

                    if (interface_name_address == nullptr) {
                        logger->warn(
                            "Failed to extract interface name address of function demux at {}",
                            (void*) function_selector_address
                        );
                    } else {
                        const String interface_name((char*) interface_name_address);

                        logger->debug("Detected interface: '{}'", interface_name);

                        // Finally, hook the selector functions of interest

                        if ("IClientAppManager" == interface_name) {
                            DETOUR_ADDRESS(IClientAppManager_Selector, function_selector_address)
                        } else if ("IClientApps" == interface_name) {
                            DETOUR_ADDRESS(IClientApps_Selector, function_selector_address)
                        } else if ("IClientInventory" == interface_name) {
                            DETOUR_ADDRESS(IClientInventory_Selector, function_selector_address)
                        } else if ("IClientUser" == interface_name) {
                            DETOUR_ADDRESS(IClientUser_Selector, function_selector_address)
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
    }
}
