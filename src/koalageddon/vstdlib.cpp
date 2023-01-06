#include <core/macros.hpp>
#include <koalageddon/koalageddon.hpp>
#include <steam_functions/steam_functions.hpp>
#include <koalabox/hook.hpp>

using namespace koalageddon;
using namespace koalabox;

typedef uint32_t HCoroutine;
DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, struct CoroutineData* data);

namespace koalageddon {
    void init_vstdlib_hooks() {
        DETOUR_VSTDLIB(Coroutine_Create)
    }
}

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
        return reinterpret_cast<FunctionAddress*>(this)[koalageddon::config.vstdlib_callback_interceptor_address_offset];
    }

    FunctionAddress get_callback_address() {
        return reinterpret_cast<FunctionAddress*>(this)[koalageddon::config.vstdlib_callback_address_offset];
    }
};

struct CoroutineData {
    CallbackData* get_callback_data() {
        return reinterpret_cast<CallbackData**>(this)[koalageddon::config.vstdlib_callback_data_offset];
    }

    const char* get_callback_name() {
        return reinterpret_cast<const char**>(this)[koalageddon::config.vstdlib_callback_name_offset];
    }
};

VIRTUAL(void) VStdLib_Callback_Interceptor(PARAMS(const char** p_name)) {
    GET_ORIGINAL_FUNCTION_VSTDLIB(VStdLib_Callback_Interceptor)

    VStdLib_Callback_Interceptor_o(ARGS(p_name));

    static auto lock_status_hooked = false;
    static auto stop_playing_hooked = false;

    if (lock_status_hooked && stop_playing_hooked) {
        return;
    }

    auto* const data = (CoroutineData*) THIS;

    if (data && data->get_callback_name()) {
        const auto name = String(data->get_callback_name());
        // logger->trace("{} -> instance: {}, name: '{}'", __func__, fmt::ptr(THIS), name);


        if (name == "SharedLicensesLockStatus" && !lock_status_hooked) {
            DETOUR_ADDRESS(SharedLicensesLockStatus, data->get_callback_data()->get_callback_address())
            lock_status_hooked = true;
        } else if (name == "SharedLibraryStopPlaying" && !stop_playing_hooked) {
            DETOUR_ADDRESS(SharedLibraryStopPlaying, data->get_callback_data()->get_callback_address())
            stop_playing_hooked = true;
        }
    }
}

/**
 * Initially, callback data passed into this function is not complete,
 * hence we must hook an interface method that sets the callback name.
 */
DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, CoroutineData* data) {
    GET_ORIGINAL_FUNCTION_VSTDLIB(Coroutine_Create)

    const auto result = Coroutine_Create_o(callback_address, data);

    // Coroutine callback appears to be always the same
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        logger->debug("Coroutine_Create -> callback: {}, data: {}", callback_address, fmt::ptr(data));

        DETOUR_ADDRESS(VStdLib_Callback_Interceptor, data->get_callback_data()->get_callback_intercept_address())
    });

    return result;
}
