#include <koalabox/hook.hpp>
#include <koalabox/patcher.hpp>
#include <steam_functions/steam_functions.hpp>
#include <Zydis/Zydis.h>
#include <Zydis/DecoderTypes.h>

namespace koalageddon::steamclient {
    using namespace koalabox;

    // map<interface name, map<function name, function ordinal>>
    Map<String, Map<String, uint32_t>> ordinal_map; // NOLINT(cert-err58-cpp)

    const auto MAX_INSTRUCTION_SIZE = 15;


    // TODO: Refactor into Koalabox
    ZydisDecoder decoder = {};
    ZydisFormatter formatter = {};

#define HOOK_FUNCTION(INTERFACE, FUNC) hook::swap_virtual_func_or_throw(    \
    globals::address_map,                                                   \
    interface,                                                              \
    #INTERFACE"_"#FUNC,                                                     \
    ordinal_map[#INTERFACE][#FUNC],                                         \
    reinterpret_cast<uintptr_t>(INTERFACE##_##FUNC)                         \
);

    DLL_EXPORT(void) IClientAppManager_Selector(
        const void* interface,
        const void* arg2,
        const void* arg3,
        const void* arg4
    ) {
        static std::once_flag flag;
        std::call_once(
            flag, [&]() {
                HOOK_FUNCTION(IClientAppManager, IsAppDlcInstalled)
            }
        );

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
        std::call_once(
            flag, [&]() {
                HOOK_FUNCTION(IClientApps, GetDLCCount)
                HOOK_FUNCTION(IClientApps, BGetDLCDataByIndex)
            }
        );

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
        std::call_once(
            flag, [&]() {
                HOOK_FUNCTION(IClientInventory, GetResultStatus)
                HOOK_FUNCTION(IClientInventory, GetResultItems)
                HOOK_FUNCTION(IClientInventory, GetResultItemProperty)
                HOOK_FUNCTION(IClientInventory, CheckResultSteamID)
                HOOK_FUNCTION(IClientInventory, GetAllItems)
                HOOK_FUNCTION(IClientInventory, GetItemsByID)
                HOOK_FUNCTION(IClientInventory, SerializeResult)
                HOOK_FUNCTION(IClientInventory, GetItemDefinitionIDs)
            }
        );

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
        std::call_once(
            flag, [&]() {
                HOOK_FUNCTION(IClientUser, BIsSubscribedApp)
            }
        );

        GET_ORIGINAL_HOOKED_FUNCTION(IClientUser_Selector)
        IClientUser_Selector_o(interface, arg2, arg3, arg4);
    }

    uintptr_t get_absolute_address(ZydisDecodedInstruction instruction, uintptr_t address) {
        const auto op = instruction.operands[0];

        if (op.imm.is_relative) {
            ZyanU64 absolute_address;
            ZydisCalcAbsoluteAddress(&instruction, &op, address, &absolute_address);

            return absolute_address;
        }

        return (uintptr_t) op.imm.value.u;
    }

    bool is_push_immediate(const ZydisDecodedInstruction& instruction) {
        const auto& operand = instruction.operands[0];

        return instruction.mnemonic == ZYDIS_MNEMONIC_PUSH &&
               operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
               operand.visibility == ZYDIS_OPERAND_VISIBILITY_EXPLICIT &&
               operand.encoding == ZYDIS_OPERAND_ENCODING_SIMM16_32_32;
    }

    std::optional<String> get_string_argument(const ZydisDecodedInstruction& instruction) {
        const auto* name_address = reinterpret_cast<char*>(instruction.operands[0].imm.value.u);
        if (util::is_valid_pointer(name_address)) {
            return name_address;
        }
        return std::nullopt;
    }

    std::optional<String> get_instruction_string(
        const ZydisDecodedInstruction& instruction,
        const uintptr_t address
    ) {
        const auto buffer_size = 64;
        char buffer[buffer_size] = {};

        if (ZYAN_SUCCESS(
            ZydisFormatterFormatInstruction(
                &formatter,
                &instruction,
                buffer,
                buffer_size,
                address
            )
        )) {
            return buffer;
        }

        return std::nullopt;
    }

    struct InstructionContext {
        std::optional<ZydisRegister> base_register;
        std::optional<String> function_name;
    };

    void construct_ordinal_map( // NOLINT(misc-no-recursion)
        const String& target_interface,
        Map<String, uint32_t>& map,
        uintptr_t start_address,
        Set<uintptr_t>& visited_addresses,
        InstructionContext context
    ) {
        if (visited_addresses.contains(start_address)) {
            // Avoid infinite recursion
            return;
        }

        visited_addresses.insert(start_address);

        if (context.function_name && map.contains(*context.function_name)) {
            // Avoid duplicate work
            return;
        }

        const auto is_mov_base_esp = [](const ZydisDecodedInstruction& instruction) {
            return instruction.mnemonic == ZYDIS_MNEMONIC_MOV &&
                   instruction.operand_count == 2 &&
                   instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
                   instruction.operands[1].reg.value == ZYDIS_REGISTER_ESP;
        };

        // Initialize with a dummy previous instruction
        std::list instruction_list{ZydisDecodedInstruction{}};

        auto current_address = (uintptr_t) start_address;
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(
            ZydisDecoderDecodeBuffer(
                &decoder,
                (void*) current_address,
                MAX_INSTRUCTION_SIZE,
                &instruction
            )
        )) {
            TRACE(
                "{} -> Visiting {} | {}",
                __func__, (void*) current_address, *get_instruction_string(instruction, current_address)
            )

            const auto& last_instruction = instruction_list.front();

            if (!context.base_register && is_mov_base_esp(instruction)) {
                // Save base register
                context.base_register = instruction.operands[0].reg.value;
            } else if (is_push_immediate(last_instruction) &&
                       is_push_immediate(instruction) &&
                       !context.function_name) {
                // The very first 2 consecutive pushes indicate interface and function names.
                // However, subsequent pushes may contain irrelevant strings.
                const auto push_string_1 = get_string_argument(last_instruction);
                const auto push_string_2 = get_string_argument(instruction);

                if (push_string_1 && push_string_2) {
                    if (*push_string_1 == target_interface) {
                        context.function_name = push_string_2;
                    } else if (*push_string_2 == target_interface) {
                        context.function_name = push_string_1;
                    }

                    if (map.contains(*context.function_name)) {
                        // Bail early to avoid duplicate work
                        return;
                    }
                }
            } else if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR) {
                // On conditional jump we should recurse
                const auto jump_taken_destination = get_absolute_address(instruction, current_address);
                const auto jump_not_taken_destination = current_address + instruction.length;

                construct_ordinal_map(target_interface, map, jump_taken_destination, visited_addresses, context);
                construct_ordinal_map(target_interface, map, jump_not_taken_destination, visited_addresses, context);

                // But not continue forward, in order to avoid duplicate processing
                return;
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP &&
                       instruction.operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
                // On unconditional jump we should recurse as well
                const auto jump_destination = get_absolute_address(instruction, current_address);

                construct_ordinal_map(target_interface, map, jump_destination, visited_addresses, context);
                return;
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL) {
                // On call instructions we should extract the ordinal

                if (context.base_register && context.function_name) {
                    std::optional<uint32_t> offset;

                    const auto operand = instruction.operands[0];

                    auto last_destination_reg = ZYDIS_REGISTER_NONE;
                    bool is_derived_from_base_reg = false;

                    // Sometimes the offset is present in the call instruction itself,
                    // hence we can immediately obtain it.
                    if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY && operand.mem.base != ZYDIS_REGISTER_NONE) {
                        offset = static_cast<uint32_t>(operand.mem.disp.value);
                        last_destination_reg = operand.mem.base;
                    } else if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
                        last_destination_reg = operand.reg.value;
                    }

                    for (const auto& previous_instruction: instruction_list) {
                        const auto& destination_operand = previous_instruction.operands[0];
                        const auto& source_operand = previous_instruction.operands[1];

                        // Extract offset if necessary
                        if (previous_instruction.mnemonic == ZYDIS_MNEMONIC_MOV &&
                            previous_instruction.operand_count == 2 &&
                            destination_operand.reg.value == last_destination_reg &&
                            source_operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {

                            const auto source_mem = source_operand.mem;
                            if (source_mem.base == *context.base_register &&
                                source_mem.disp.has_displacement &&
                                source_mem.disp.value == 8) {
                                // We have verified that the chain eventually leads up to the base register.
                                // Hence, we can conclude that the offset is valid.
                                is_derived_from_base_reg = true;
                                break;
                            }

                            // Otherwise, keep going through the chain
                            last_destination_reg = source_mem.base;

                            if (!offset) {
                                offset = static_cast<uint32_t>(source_mem.disp.value);
                            }
                        }
                    }

                    if (offset && is_derived_from_base_reg) {
                        const auto ordinal = *offset / sizeof(uintptr_t);

                        logger->debug(
                            "{} -> Found function ordinal {}::{}@{}",
                            __func__, target_interface, *context.function_name, ordinal
                        );

                        map[*context.function_name] = ordinal;
                        break;
                    }
                }
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_RET) {
                // Finish parsing on return
                return;
            }

            // We push items to the front so that it becomes easy to iterate over instructions
            // in reverse order of addition.
            instruction_list.push_front(instruction);
            current_address += instruction.length;
        }
    }

    std::optional<String> find_interface_name(uintptr_t selector_address) {
        auto current_address = selector_address;
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(
            ZydisDecoderDecodeBuffer(
                &decoder,
                (void*) current_address,
                MAX_INSTRUCTION_SIZE,
                &instruction
            )
        )) {
            const auto debug_str = get_instruction_string(instruction, current_address);

            if (is_push_immediate(instruction)) {
                auto string_opt = get_string_argument(instruction);

                if (string_opt && string_opt->starts_with("IClient")) {
                    return string_opt;
                }
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_RET) {
                break;
            }

            current_address += instruction.length;
        }

        // logger->warn("Failed to find any interface names at {}", (void*) selector_address);

        return std::nullopt;
    }

#define CONSTRUCT_ORDINAL_MAP(INTERFACE)        \
    Set<uintptr_t> nested_visited_addresses;    \
    construct_ordinal_map(                      \
        #INTERFACE,                             \
        ordinal_map[#INTERFACE],                \
        function_selector_address,              \
        nested_visited_addresses,               \
        {}                                      \
    );

#define DETOUR_SELECTOR(INTERFACE)                                      \
    if(interface_name == #INTERFACE){                                   \
        CONSTRUCT_ORDINAL_MAP(INTERFACE)                                \
        DETOUR_ADDRESS(INTERFACE##_Selector, function_selector_address) \
    }

    void process_interface_selector( // NOLINT(misc-no-recursion)
        const uintptr_t start_address,
        Set<uintptr_t>& visited_addresses
    ) {
        TRACE("{} -> start_address: {}", __func__, (void*) start_address)

        if (visited_addresses.contains(start_address)) {
            TRACE("{} -> Breaking recursion due to visited address", __func__)
            return;
        }

        auto current_address = start_address;

        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(
            ZydisDecoderDecodeBuffer(
                &decoder,
                (void*) current_address,
                MAX_INSTRUCTION_SIZE,
                &instruction
            )
        )) {
            visited_addresses.insert(current_address);
            TRACE(
                "{} -> Visiting {} | {}",
                __func__, (void*) current_address, *get_instruction_string(instruction, current_address)
            )

            const auto operand = instruction.operands[0];

            if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL &&
                operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE
                ) {
                TRACE("{} -> Found call instruction at {}", __func__, (void*) current_address)

                const auto function_selector_address = get_absolute_address(instruction, current_address);

                const auto interface_name_opt = find_interface_name(function_selector_address);

                if (interface_name_opt) {
                    const auto& interface_name = *interface_name_opt;

                    logger->debug("Detected interface: '{}'", interface_name);

                    DETOUR_SELECTOR(IClientAppManager)
                    DETOUR_SELECTOR(IClientApps)
                    DETOUR_SELECTOR(IClientInventory)
                    DETOUR_SELECTOR(IClientUser)
                }
            } else if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR) {
                const auto jump_taken_destination = get_absolute_address(instruction, current_address);
                const auto jump_not_taken_destination = current_address + instruction.length;

                process_interface_selector(jump_taken_destination, visited_addresses);
                process_interface_selector(jump_not_taken_destination, visited_addresses);

                TRACE("breaking recursion due to conditional branch")
                return;
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP &&
                       operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE
                ) {
                const auto jump_destination = get_absolute_address(instruction, current_address);

                process_interface_selector(jump_destination, visited_addresses);

                TRACE("breaking recursion due to unconditional branch")
                return;
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP &&
                       operand.type == ZYDIS_OPERAND_TYPE_MEMORY &&
                       operand.mem.scale == sizeof(uintptr_t) &&
                       operand.mem.disp.has_displacement
                ) {
                // Special handling for jump tables. Guaranteed to be present in the interface selector.
                const auto* table = (uintptr_t*) operand.mem.disp.value;

                const auto* table_entry = table;
                while (util::is_valid_pointer((void*) *table_entry)) {
                    process_interface_selector(*table_entry, visited_addresses);

                    table_entry++;
                }

                return;
            } else if (instruction.mnemonic == ZYDIS_MNEMONIC_RET) {
                TRACE("{} -> Breaking recursion due to return instruction", __func__)
                return;
            }

            current_address += instruction.length;
        }
    }

    void init(const uintptr_t interface_selector_address) {
        if (ZYAN_FAILED(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32))) {
            logger->error("Failed to initialize zydis decoder");
            return;
        }

        if (ZYAN_FAILED(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL))) {
            logger->error("Failed to initialize zydis formatter");
            return;
        }

        Set<uintptr_t> visited_addresses;
        process_interface_selector(interface_selector_address, visited_addresses);
    }
}
