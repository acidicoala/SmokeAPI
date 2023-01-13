#include <store_mode/steamclient/steamclient.hpp>
#include <store_mode/store.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>
#include <koalabox/util.hpp>

#include <Zydis/Zydis.h>
#include <Zydis/DecoderTypes.h>

namespace store::steamclient {
    using namespace koalabox;

    Map<String, void*> interface_name_to_address_map; // NOLINT(cert-err58-cpp)

    struct InstructionContext {
        std::optional<ZydisRegister> base_register;
        std::optional<String> function_name;
    };

    // map<interface name, map<function name, function ordinal>>
    Map<String, Map<String, uint32_t>> ordinal_map; // NOLINT(cert-err58-cpp)

    const auto MAX_INSTRUCTION_SIZE = 15;

    ZydisDecoder decoder = {};
    ZydisFormatter formatter = {};

    void construct_ordinal_map( // NOLINT(misc-no-recursion)
        const String& target_interface,
        Map<String, uint32_t>& function_name_to_ordinal_map,
        uintptr_t start_address
    );

#define CONSTRUCT_ORDINAL_MAP(INTERFACE) \
    construct_ordinal_map(#INTERFACE, ordinal_map[#INTERFACE], function_selector_address);

#define HOOK_FUNCTION(INTERFACE, FUNC) hook::swap_virtual_func_or_throw( \
    globals::address_map, \
    interface, \
    #INTERFACE"_"#FUNC, \
    ordinal_map[#INTERFACE][#FUNC], \
    reinterpret_cast<uintptr_t>(INTERFACE##_##FUNC) \
);

#define SELECTOR_IMPLEMENTATION(INTERFACE, FUNC_BODY) \
    DLL_EXPORT(void) INTERFACE##_Selector( \
        void* interface, \
        void* arg2, \
        void* arg3, \
        void* arg4 \
    ) { \
        CALL_ONCE({ \
            interface_name_to_address_map[#INTERFACE] = interface; \
            [&]()FUNC_BODY(); \
        }) \
        GET_ORIGINAL_HOOKED_FUNCTION(INTERFACE##_Selector) \
        INTERFACE##_Selector_o(interface, arg2, arg3, arg4); \
    }

    SELECTOR_IMPLEMENTATION(IClientAppManager, {
        HOOK_FUNCTION(IClientAppManager, IsAppDlcInstalled)
    })

    SELECTOR_IMPLEMENTATION(IClientApps, {
        HOOK_FUNCTION(IClientApps, GetDLCCount)
        HOOK_FUNCTION(IClientApps, BGetDLCDataByIndex)
    })

    SELECTOR_IMPLEMENTATION(IClientInventory, {
        HOOK_FUNCTION(IClientInventory, GetResultStatus)
        HOOK_FUNCTION(IClientInventory, GetResultItems)
        HOOK_FUNCTION(IClientInventory, GetResultItemProperty)
        HOOK_FUNCTION(IClientInventory, CheckResultSteamID)
        HOOK_FUNCTION(IClientInventory, GetAllItems)
        HOOK_FUNCTION(IClientInventory, GetItemsByID)
        HOOK_FUNCTION(IClientInventory, SerializeResult)
        HOOK_FUNCTION(IClientInventory, GetItemDefinitionIDs)
    })

    SELECTOR_IMPLEMENTATION(IClientUser, {
        HOOK_FUNCTION(IClientUser, BIsSubscribedApp)
    })

    SELECTOR_IMPLEMENTATION(IClientUtils, {
        HOOK_FUNCTION(IClientUtils, GetAppID)
    })

#define DETOUR_SELECTOR(INTERFACE)                                      \
    if(interface_name == #INTERFACE){                                   \
        CONSTRUCT_ORDINAL_MAP(INTERFACE)                                \
        DETOUR_ADDRESS(INTERFACE##_Selector, function_selector_address) \
    }

    void detour_interface_selector(const String& interface_name, uintptr_t function_selector_address) {
        LOG_DEBUG("Detected interface: '{}'", interface_name)

        DETOUR_SELECTOR(IClientAppManager)
        DETOUR_SELECTOR(IClientApps)
        DETOUR_SELECTOR(IClientInventory)
        DETOUR_SELECTOR(IClientUser)
        DETOUR_SELECTOR(IClientUtils)
    }

    uintptr_t get_absolute_address(ZydisDecodedInstruction instruction, uintptr_t address) {
        const auto operand = instruction.operands[0];

        if (operand.imm.is_relative) {
            ZyanU64 absolute_address;
            ZydisCalcAbsoluteAddress(&instruction, &operand, address, &absolute_address);

            return absolute_address;
        }

        return (uintptr_t) operand.imm.value.u;
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

    std::optional<String> find_interface_name(uintptr_t selector_address) {
        auto current_address = selector_address;
        ZydisDecodedInstruction instruction{};
        while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(
            &decoder,
            (void*) current_address,
            MAX_INSTRUCTION_SIZE,
            &instruction
        ))) {
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

        // LOG_WARN("Failed to find any interface names at {}", (void*) selector_address);

        return std::nullopt;
    }

    /**
     * Recursively walks through the code, until a return instruction is reached.
     * Recursion occurs whenever a jump/branch is encountered.
     */
    template<typename T>
    void visit_code( // NOLINT(misc-no-recursion)
        Set<uintptr_t>& visited_addresses,
        uintptr_t start_address,
        T context,
        const Function<bool(
            const ZydisDecodedInstruction& instruction,
            const ZydisDecodedOperand& operand,
            const uintptr_t& current_address,
            T& context,
            const std::list<ZydisDecodedInstruction>& instruction_list
        )>& callback
    ) {
        LOG_TRACE("{} -> start_address: {}", __func__, (void*) start_address)

        if (visited_addresses.contains(start_address)) {
            LOG_TRACE("Breaking recursion due to visited address")
            return;
        }

        auto current_address = start_address;
        std::list instruction_list{ZydisDecodedInstruction{}};

        ZydisDecodedInstruction instruction{};
        while (
            ZYAN_SUCCESS(
                ZydisDecoderDecodeBuffer(
                    &decoder,
                    (void*) current_address,
                    MAX_INSTRUCTION_SIZE,
                    &instruction
                )
            )) {
            visited_addresses.insert(current_address);
            LOG_TRACE(
                "{} -> visiting {} │ {}",
                __func__, (void*) current_address, *get_instruction_string(instruction, current_address)
            )

            const auto operand = instruction.operands[0];

            const auto should_return = callback(instruction, operand, current_address, context, instruction_list);

            if (should_return) {
                return;
            }

            if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR) {
                const auto jump_taken_destination = get_absolute_address(instruction, current_address);
                const auto jump_not_taken_destination = current_address + instruction.length;

                visit_code(visited_addresses, jump_taken_destination, context, callback);
                visit_code(visited_addresses, jump_not_taken_destination, context, callback);

                LOG_TRACE("{} -> Breaking recursion due to a conditional branch", __func__)
                return;
            }

            if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP && operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
                const auto jump_destination = get_absolute_address(instruction, current_address);

                visit_code(visited_addresses, jump_destination, context, callback);

                LOG_TRACE("{} -> Breaking recursion due to an unconditional jump", __func__)
                return;
            }

            if (instruction.mnemonic == ZYDIS_MNEMONIC_JMP &&
                operand.type == ZYDIS_OPERAND_TYPE_MEMORY &&
                operand.mem.scale == sizeof(uintptr_t) &&
                operand.mem.disp.has_displacement) {
                // Special handling for jump tables. Guaranteed to be present in the interface selector.
                const auto* table = (uintptr_t*) operand.mem.disp.value;

                const auto* table_entry = table;
                while (util::is_valid_pointer((void*) *table_entry)) {
                    visit_code(visited_addresses, *table_entry, context, callback);

                    table_entry++;
                }

                LOG_TRACE("{} -> Breaking recursion due to a jump table", __func__)
                return;
            }

            if (instruction.mnemonic == ZYDIS_MNEMONIC_RET) {
                LOG_TRACE("{} -> Breaking recursion due to return instruction", __func__)
                return;
            }


            // We push items to the front so that it becomes easy to iterate over instructions
            // in reverse order of addition.
            instruction_list.push_front(instruction);
            current_address += instruction.length;
        }
    }

    void construct_ordinal_map( // NOLINT(misc-no-recursion)
        const String& target_interface,
        Map<String, uint32_t>& function_name_to_ordinal_map,
        uintptr_t start_address
    ) {
        Set<uintptr_t> visited_addresses;
        visit_code<InstructionContext>(visited_addresses, start_address, {}, [&](
                const ZydisDecodedInstruction& instruction,
                const ZydisDecodedOperand& operand,
                const auto&,
                InstructionContext& context,
                const std::list<ZydisDecodedInstruction>& instruction_list
            ) {
                if (context.function_name && function_name_to_ordinal_map.contains(*context.function_name)) {
                    // Avoid duplicate work
                    return true;
                }

                const auto& last_instruction = instruction_list.front();

                const auto is_mov_base_esp = instruction.mnemonic == ZYDIS_MNEMONIC_MOV &&
                                             instruction.operand_count == 2 &&
                                             instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
                                             instruction.operands[1].reg.value == ZYDIS_REGISTER_ESP;

                if (!context.base_register && is_mov_base_esp) {
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

                        if (context.function_name && function_name_to_ordinal_map.contains(*context.function_name)) {
                            // Bail early to avoid duplicate work
                            return true;
                        }
                    }
                } else if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL) {
                    // On call instructions we should extract the ordinal

                    if (context.base_register && context.function_name) {
                        const auto& base_register = *(context.base_register);
                        const auto& function_name = *(context.function_name);


                        std::optional<uint32_t> offset;

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
                                if (source_mem.base == base_register &&
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

                            LOG_DEBUG("Found function ordinal {}::{}@{}", target_interface, function_name, ordinal)

                            function_name_to_ordinal_map[function_name] = ordinal;
                            return true;
                        }
                    }
                }

                return false;
            }
        );
    }

    void process_interface_selector( // NOLINT(misc-no-recursion)
        const uintptr_t start_address,
        Set<uintptr_t>& visited_addresses
    ) {
        visit_code<nullptr_t>(visited_addresses, start_address, nullptr, [](
                const ZydisDecodedInstruction& instruction,
                const ZydisDecodedOperand& operand,
                const auto& current_address,
                auto,
                const auto&
            ) {
                if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL && operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
                    LOG_TRACE("Found call instruction at {}", (void*) current_address)

                    const auto function_selector_address = get_absolute_address(instruction, current_address);

                    const auto interface_name_opt = find_interface_name(function_selector_address);

                    if (interface_name_opt) {
                        const auto& interface_name = *interface_name_opt;

                        detour_interface_selector(interface_name, function_selector_address);
                    }
                }

                return false;
            }
        );
    }

    void process_client_engine(uintptr_t interface) {
        const auto* steam_client_internal = ((uintptr_t***) interface)[
            store::config.client_engine_steam_client_internal_ordinal
        ];
        const auto interface_selector_address = (*steam_client_internal)[
            store::config.steam_client_internal_interface_selector_ordinal
        ];

        LOG_DEBUG("Found interface selector at: {}", (void*) interface_selector_address)

        if (ZYAN_FAILED(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32))) {
            LOG_ERROR("Failed to initialize zydis decoder")
            return;
        }

        if (ZYAN_FAILED(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL))) {
            LOG_ERROR("Failed to initialize zydis formatter")
            return;
        }

        Set<uintptr_t> visited_addresses;
        process_interface_selector(interface_selector_address, visited_addresses);
    }

}
