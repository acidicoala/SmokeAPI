#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>

#include <koalabox/hook.hpp>

using namespace smoke_api;

#define DETOUR(FUNC, address) hook::detour((FunctionAddress) (address), #FUNC, (FunctionAddress) (FUNC));

VIRTUAL(bool) SharedLicensesLockStatus(PARAMS(void* arg)) { // NOLINT(misc-unused-parameters)
    logger->debug("{} -> instance: {}, arg: {}", __func__, fmt::ptr(THIS), fmt::ptr(arg));
    return true;
}

VIRTUAL(bool) SharedLibraryStopPlaying(PARAMS(void* arg)) { // NOLINT(misc-unused-parameters)
    logger->debug("{} -> instance: {}, arg: {}", __func__, fmt::ptr(THIS), fmt::ptr(arg));
    return true;
}

struct CallbackData {
    [[maybe_unused]] void* pad1[1];
    void* set_callback_name_address;
    [[maybe_unused]] void* pad15[15];
    void* callback_address;
};

struct CoroutineData {
    CallbackData* callback_data;
    [[maybe_unused]] uint32_t pad3[3];
    const char* callback_name;
};

VIRTUAL(void) set_callback_name(PARAMS(const char** p_name)) {
    GET_ORIGINAL_FUNCTION(set_callback_name)

    set_callback_name_o(ARGS(p_name));

    static auto hooked_functions = 0;

    if (hooked_functions == 2) {
        return;
    }

    auto* const data = (CoroutineData*) THIS;

    if (data && data->callback_name) {
        const auto name = String(data->callback_name);
        // logger->trace("{} -> instance: {}, name: '{}'", __func__, fmt::ptr(THIS), name);

        if (name == "SharedLicensesLockStatus") {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                DETOUR(SharedLicensesLockStatus, data->callback_data->callback_address)
                hooked_functions++;
            });
        } else if (name == "SharedLibraryStopPlaying") {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                DETOUR(SharedLibraryStopPlaying, data->callback_data->callback_address)
                hooked_functions++;
            });
        }
    }
}

/**
 * Initially, callback data passed into this function is not complete,
 * hence we must hook an interface method that sets the callback name.
 */
DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, CoroutineData* data) {
    GET_ORIGINAL_FUNCTION(Coroutine_Create)

    const auto result = Coroutine_Create_o(callback_address, data);

    // Coroutine callback appears to be always the same
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        logger->debug("Coroutine_Create -> callback: {}, data: {}", callback_address, fmt::ptr(data));
        DETOUR(set_callback_name, data->callback_data->set_callback_name_address)
    });

    return result;
}
