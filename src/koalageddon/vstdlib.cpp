#include <koalageddon/vstdlib.hpp>
#include <core/macros.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/logger.hpp>

using namespace koalageddon;

namespace koalageddon::vstdlib {
    using namespace koalabox;

    VIRTUAL(bool) SharedLicensesLockStatus(PARAMS(void* arg)) {
        LOG_DEBUG("{} -> ecx: {}, edx: {}, arg: {}", __func__, ARGS(arg))
        return true;
    }

    VIRTUAL(bool) SharedLibraryStopPlaying(PARAMS(void* arg)) {
        LOG_DEBUG("{} -> ecx: {}, edx: {}, arg: {}", __func__, ARGS(arg))
        return true;
    }

    VIRTUAL(void) VStdLib_Callback_Interceptor(PARAMS(const char** name_ptr)) {
        GET_ORIGINAL_HOOKED_FUNCTION(VStdLib_Callback_Interceptor)
        VStdLib_Callback_Interceptor_o(ARGS(name_ptr));

        static auto lock_status_hooked = false;
        static auto stop_playing_hooked = false;

        if (lock_status_hooked && stop_playing_hooked) {
            return;
        }

        auto* const data = (CoroutineData*) THIS;

        if (data && data->get_callback_name()) {
            const auto name = String(data->get_callback_name());
            LOG_TRACE("{} -> instance: {}, name: '{}'", __func__, fmt::ptr(THIS), name)

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
        GET_ORIGINAL_HOOKED_FUNCTION(Coroutine_Create)

        const auto result = Coroutine_Create_o(callback_address, data);

        // Coroutine callback appears to be always the same
        CALL_ONCE({
            LOG_DEBUG("Coroutine_Create -> callback: {}, data: {}", callback_address, fmt::ptr(data));

            DETOUR_ADDRESS(
                VStdLib_Callback_Interceptor,
                data->get_callback_data()->get_callback_intercept_address()
            )
        })

        return result;
    }

}
