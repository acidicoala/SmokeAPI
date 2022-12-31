#include <smoke_api/smoke_api.hpp>
#include <steam_functions/steam_functions.hpp>
#include <koalageddon/koalageddon.hpp>

#include <koalabox/hook.hpp>

using namespace smoke_api;

//#define DETOUR_STRICT(FUNC, address) hook::detour((FunctionAddress) (address), #FUNC, (FunctionAddress) (FUNC));

VIRTUAL(bool) SharedLicensesLockStatus(PARAMS(void* arg)) { // NOLINT(misc-unused-parameters)
    logger->debug("{} -> instance: {}, arg: {}", __func__, fmt::ptr(THIS), fmt::ptr(arg));
    return true;
}

VIRTUAL(bool) SharedLibraryStopPlaying(PARAMS(void* arg)) { // NOLINT(misc-unused-parameters)
    logger->debug("{} -> instance: {}, arg: {}", __func__, fmt::ptr(THIS), fmt::ptr(arg));
    return true;
}

struct CallbackData {
    FunctionAddress get_callback_intercept_address() {
        return reinterpret_cast<FunctionAddress*>(this)[koalageddon::config.callback_interceptor_address_offset];
    }

    FunctionAddress get_callback_address() {
        return reinterpret_cast<FunctionAddress*>(this)[koalageddon::config.callback_data_offset];
    }
};

struct CoroutineData {
    CallbackData* get_callback_data() {
        return reinterpret_cast<CallbackData**>(this)[koalageddon::config.callback_data_offset];
    }

    const char* get_callback_name() {
        return reinterpret_cast<const char**>(this)[koalageddon::config.callback_name_offset];
    }
};

VIRTUAL(void) VStdLib_Callback_Interceptor(PARAMS(const char** p_name)) {
    GET_ORIGINAL_FUNCTION(VStdLib_Callback_Interceptor)

    VStdLib_Callback_Interceptor_o(ARGS(p_name));

    static auto hooked_functions = 0;

    if (hooked_functions == 2) {
        return;
    }

    auto* const data = (CoroutineData*) THIS;

    if (data && data->get_callback_name()) {
        const auto name = String(data->get_callback_name());
        // logger->trace("{} -> instance: {}, name: '{}'", __func__, fmt::ptr(THIS), name);

        if (name == "SharedLicensesLockStatus") {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                DETOUR(SharedLicensesLockStatus, data->get_callback_data()->get_callback_address())
                hooked_functions++;
            });
        } else if (name == "SharedLibraryStopPlaying") {
            static std::once_flag flag;
            std::call_once(flag, [&]() {
                DETOUR(SharedLibraryStopPlaying, data->get_callback_data()->get_callback_address())
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

        DETOUR(VStdLib_Callback_Interceptor, data->get_callback_data()->get_callback_intercept_address())
    });

    return result;
}
