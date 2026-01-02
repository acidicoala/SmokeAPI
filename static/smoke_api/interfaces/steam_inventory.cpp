#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_inventory.hpp"
#include "smoke_api/config.hpp"

namespace smoke_api::steam_inventory {
    EResult GetResultStatus(
        const std::string& function_name,
        const SteamInventoryResult_t resultHandle,
        const std::function<EResult()>& original_function
    ) noexcept {
        try {
            const auto status = original_function();

            LOG_DEBUG(
                "{} -> handle: {}, status: {}",
                function_name,
                resultHandle,
                static_cast<int>(status)
            );

            return status;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return EResult::k_EResultFail;
        }
    }

    bool GetResultItems(
        const std::string& function_name,
        const SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize,
        const std::function<bool()>& original_function,
        const std::function<bool(SteamItemDef_t*, uint32_t*)>& get_item_definition_ids
    ) noexcept {
        try {
            static std::mutex section;
            const std::lock_guard guard(section);

            const auto success = original_function();

            auto print_item = [](const std::string& tag, const SteamItemDetails_t& item) {
                LOG_DEBUG(
                    "  [{}] definitionId: {}, itemId: {}, quantity: {}, flags: {}",
                    tag,
                    item.m_iDefinition,
                    item.m_itemId,
                    item.m_unQuantity,
                    item.m_unFlags
                );
            };

            if(not success) {
                LOG_DEBUG("{} -> original result is false", function_name);
                return success;
            }

            if(punOutItemsArraySize == nullptr) {
                LOG_ERROR("{} -> arraySize pointer is null", function_name);
                return success;
            }

            LOG_DEBUG(
                "{} -> handle: {}, pOutItemsArray: {}, arraySize: {}",
                function_name,
                resultHandle,
                reinterpret_cast<void*>(pOutItemsArray),
                *punOutItemsArraySize
            );

            static uint32_t original_count = 0;
            const auto injected_count = config::get().extra_inventory_items.size();

            // Automatically get inventory items from steam
            static std::vector<SteamItemDef_t> auto_inventory_items;
            if(config::get().auto_inject_inventory) {
                static std::once_flag inventory_inject_flag;
                std::call_once(
                    inventory_inject_flag,
                    [&] {
                        uint32_t count = 0;
                        if(get_item_definition_ids(nullptr, &count)) {
                            auto_inventory_items.resize(count);
                            get_item_definition_ids(auto_inventory_items.data(), &count);
                        }
                    }
                );
            }

            const auto auto_injected_count = auto_inventory_items.size();

            if(not pOutItemsArray) {
                // If pOutItemsArray is NULL then we must set the array size.
                original_count = *punOutItemsArraySize;
                *punOutItemsArraySize += auto_injected_count + injected_count;
                LOG_DEBUG(
                    "{} -> Original count: {}, Total count: {}",
                    function_name,
                    original_count,
                    *punOutItemsArraySize
                );
            } else {
                // Otherwise, we modify the array
                for(int i = 0; i < original_count; i++) {
                    print_item("original", pOutItemsArray[i]);
                }

                static auto new_item = [](SteamItemDef_t id) {
                    return SteamItemDetails_t{
                        .m_itemId = id,
                        .m_iDefinition = id,
                        .m_unQuantity = 1,
                        .m_unFlags = 0,
                    };
                };

                for(int i = 0; i < auto_injected_count; i++) {
                    auto& item = pOutItemsArray[original_count + i];
                    const auto item_def_id = auto_inventory_items[i];

                    item = new_item(item_def_id);

                    print_item("auto-injected", item);
                }

                for(int i = 0; i < injected_count; i++) {
                    auto& item = pOutItemsArray[original_count + auto_injected_count + i];
                    const auto item_def_id = config::get().extra_inventory_items[i];

                    item = new_item(item_def_id);

                    print_item("injected", item);
                }
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool GetResultItemProperty(
        const std::string& function_name,
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        const char* pchValueBuffer,
        const uint32_t* punValueBufferSizeOut,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            LOG_DEBUG(
                "{} -> Handle: {}, Index: {}, Name: '{}'",
                function_name,
                resultHandle,
                unItemIndex,
                // can be empty, in which case steam responds with property list in csv format
                pchPropertyName ? pchPropertyName : "nullptr"
            );

            const auto success = original_function();

            if(!success) {
                LOG_WARN("{} -> Result is false", function_name);
                return false;
            }

            if(
                pchValueBuffer && *pchValueBuffer &&
                punValueBufferSizeOut && *punValueBufferSizeOut > 0
            ) {
                LOG_DEBUG(
                    R"('{}' -> Buffer: "{}")",
                    function_name,
                    std::string(pchValueBuffer, *punValueBufferSizeOut - 1)
                );
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool GetAllItems(
        const std::string& function_name,
        const SteamInventoryResult_t* pResultHandle,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto success = original_function();

            LOG_DEBUG(
                "{} -> Handle: {}",
                function_name,
                reinterpret_cast<const void*>(pResultHandle)
            );

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool GetItemsByID(
        const std::string& function_name,
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        const uint32_t unCountInstanceIDs,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto success = original_function();

            LOG_DEBUG("{} -> Handle: {}", function_name, static_cast<void*>(pResultHandle));

            if(success && pInstanceIDs != nullptr) {
                for(int i = 0; i < unCountInstanceIDs; i++) {
                    LOG_DEBUG("  Index: {}, ItemId: {}", i, pInstanceIDs[i]);
                }
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool SerializeResult(
        const std::string& function_name,
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t* punOutBufferSize,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto success = original_function();

            if(pOutBuffer != nullptr) {
                std::string buffer(static_cast<char*>(pOutBuffer), *punOutBufferSize);
                LOG_DEBUG("{} -> Handle: {}, Buffer: '{}'", function_name, resultHandle, buffer);
            } else {
                LOG_DEBUG(
                    "{} -> Handle: {}, Size: '{}'",
                    function_name,
                    resultHandle,
                    *punOutBufferSize
                );
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool GetItemDefinitionIDs(
        const std::string& function_name,
        const SteamItemDef_t* pItemDefIDs,
        uint32_t* punItemDefIDsArraySize,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto success = original_function();

            if(!success) {
                LOG_WARN("{} -> Result is false", function_name);
                return false;
            }

            if(punItemDefIDsArraySize) {
                LOG_DEBUG("{} -> Size: {}", function_name, *punItemDefIDsArraySize);
            } else {
                return success;
            }

            if(pItemDefIDs) { // Definitions were copied
                for(int i = 0; i < *punItemDefIDsArraySize; i++) {
                    const auto& def = pItemDefIDs[i];
                    LOG_DEBUG("  Index: {}, ID: {}", i, def);
                }
            }

            return success;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }

    bool CheckResultSteamID(
        const std::string& function_name,
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto result = original_function();

            LOG_DEBUG(
                "{} -> handle: {}, steamID: {}, original result: {}",
                function_name,
                resultHandle,
                steamIDExpected,
                result
            );

            return true;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", function_name, e.what());
            return false;
        }
    }
}
