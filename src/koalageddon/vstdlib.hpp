#include <koalageddon/koalageddon.hpp>

namespace koalageddon::vstdlib {

    struct CallbackData {
        uintptr_t get_callback_intercept_address() {
            return reinterpret_cast<uintptr_t*>(this)[koalageddon::config.vstdlib_callback_interceptor_address_offset];
        }

        uintptr_t get_callback_address() {
            return reinterpret_cast<uintptr_t*>(this)[koalageddon::config.vstdlib_callback_address_offset];
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

    typedef uint32_t HCoroutine;
    DLL_EXPORT(HCoroutine) Coroutine_Create(void* callback_address, CoroutineData* data);
}
