#include <smoke_api/smoke_api.hpp>
#include <steam_impl/steam_inventory.hpp>

namespace steam_inventory {

    EResult GetResultStatus(
        const String& function_name,
        const SteamInventoryResult_t resultHandle,
        const std::function<EResult()>& original_function
    ) {
        const auto status = original_function();

        logger->debug("{} -> handle: {}, status: {}", function_name, resultHandle, (int) status);

        return status;
    }

    bool GetResultItems(
        const String& function_name,
        const SteamInventoryResult_t resultHandle,
        SteamItemDetails_t* pOutItemsArray,
        uint32_t* punOutItemsArraySize,
        const std::function<bool()>& original_function,
        const std::function<bool(SteamItemDef_t*, uint32_t*)>& get_item_definition_ids
    ) {
        static std::mutex section;
        const std::lock_guard<std::mutex> guard(section);

        const auto success = original_function();

        auto print_item = [](const String& tag, const SteamItemDetails_t& item) {
            logger->debug(
                "  [{}] definitionId: {}, itemId: {}, quantity: {}, flags: {}",
                tag, item.m_iDefinition, item.m_itemId, item.m_unQuantity, item.m_unFlags
            );
        };

        if (not success) {
            logger->debug("{} -> original result is false", function_name);
            return success;
        }

        if (punOutItemsArraySize == nullptr) {
            logger->error("{} -> arraySize pointer is null", function_name);
            return success;
        }

        logger->debug(
            "{} -> handle: {}, pOutItemsArray: {}, arraySize: {}",
            function_name, resultHandle, fmt::ptr(pOutItemsArray), *punOutItemsArraySize
        );

        static uint32_t original_count = 0;
        const auto injected_count = smoke_api::config.inventory_items.size();

        // Automatically get inventory items from steam
        static Vector<SteamItemDef_t> auto_inventory_items;
        if (smoke_api::config.auto_inject_inventory) {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                uint32_t count = 0;
                if (get_item_definition_ids(nullptr, &count)) {
                    auto_inventory_items.resize(count);
                    get_item_definition_ids(auto_inventory_items.data(), &count);
                }
            });
        }

        const auto auto_injected_count = auto_inventory_items.size();

        if (not pOutItemsArray) {
            // If pOutItemsArray is NULL then we must set the array size.
            original_count = *punOutItemsArraySize;
            *punOutItemsArraySize += auto_injected_count + injected_count;
            logger->debug(
                "{} -> Original count: {}, Total count: {}",
                function_name, original_count, *punOutItemsArraySize
            );
        } else {
            // Otherwise, we modify the array
            for (int i = 0; i < original_count; i++) {
                print_item("original", pOutItemsArray[i]);
            }

            static auto new_item = [](SteamItemDef_t id) {
                return SteamItemDetails_t{
                    .m_itemId=id,
                    .m_iDefinition=id,
                    .m_unQuantity=1,
                    .m_unFlags=0,
                };
            };

            for (int i = 0; i < auto_injected_count; i++) {
                auto& item = pOutItemsArray[original_count + i];
                const auto item_def_id = auto_inventory_items[i];

                item = new_item(item_def_id);

                print_item("auto-injected", item);
            }

            for (int i = 0; i < injected_count; i++) {
                auto& item = pOutItemsArray[original_count + auto_injected_count + i];
                const auto item_def_id = smoke_api::config.inventory_items[i];

                item = new_item(item_def_id);

                print_item("injected", item);
            }
        }

        return success;
    }

    bool GetResultItemProperty(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        uint32_t unItemIndex,
        const char* pchPropertyName,
        char* pchValueBuffer,
        const uint32_t* punValueBufferSizeOut,
        const std::function<bool()>& original_function
    ) {
        const auto common_info = fmt::format(
            "{} -> Handle: {}, Index: {}, Name: '{}'", function_name, resultHandle, unItemIndex, pchPropertyName
        );

        const auto success = original_function();

        if (!success) {
            logger->warn("{}, Result is false", common_info);
            return false;
        }

        logger->debug("{}, Buffer: '{}'", common_info, String(pchValueBuffer, *punValueBufferSizeOut - 1));

        return success;
    }

    bool GetAllItems(
        const String& function_name,
        const SteamInventoryResult_t* pResultHandle,
        const std::function<bool()>& original_function
    ) {
        const auto success = original_function();

        logger->debug("{} -> Handle: {}", function_name, fmt::ptr(pResultHandle));

        return success;
    }


    bool GetItemsByID(
        const String& function_name,
        SteamInventoryResult_t* pResultHandle,
        const SteamItemInstanceID_t* pInstanceIDs,
        const uint32_t unCountInstanceIDs,
        const std::function<bool()>& original_function
    ) {
        const auto success = original_function();

        logger->debug("{} -> Handle: {}", function_name, fmt::ptr(pResultHandle));

        if (success && pInstanceIDs != nullptr) {
            for (int i = 0; i < unCountInstanceIDs; i++) {
                logger->debug("  Index: {}, ItemId: {}", i, pInstanceIDs[i]);
            }
        }

        return success;
    }

    bool SerializeResult(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        void* pOutBuffer,
        uint32_t* punOutBufferSize,
        const std::function<bool()>& original_function
    ) {
        const auto success = original_function();

        if (pOutBuffer != nullptr) {
            String buffer((char*) pOutBuffer, *punOutBufferSize);
            logger->debug("{} -> Handle: {}, Buffer: '{}'", function_name, resultHandle, buffer);
        } else {
            logger->debug("{} -> Handle: {}, Size: '{}'", function_name, resultHandle, *punOutBufferSize);
        }

        return success;
    }

    bool GetItemDefinitionIDs(
        const String& function_name,
        const SteamItemDef_t* pItemDefIDs,
        uint32_t* punItemDefIDsArraySize,
        const std::function<bool()>& original_function
    ) {
        const auto success = original_function();

        if (!success) {
            logger->warn("{} -> Result is false", function_name);
            return false;
        }

        if (punItemDefIDsArraySize) {
            logger->debug("{} -> Size: {}", function_name, *punItemDefIDsArraySize);
        }

        if (pItemDefIDs) { // Definitions were copied
            for (int i = 0; i < *punItemDefIDsArraySize; i++) {
                const auto& def = pItemDefIDs[i];
                logger->debug("  Index: {}, ID: {}", i, def);
            }
        }

        return success;
    }

    bool CheckResultSteamID(
        const String& function_name,
        SteamInventoryResult_t resultHandle,
        CSteamID steamIDExpected,
        const std::function<bool()>& original_function
    ) {
        const auto result = original_function();

        logger->debug(
            "{} -> handle: {}, steamID: {}, original result: {}",
            function_name, resultHandle, steamIDExpected, result
        );

        return true;
    }
}
