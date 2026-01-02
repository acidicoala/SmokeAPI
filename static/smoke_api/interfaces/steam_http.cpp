#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_http.hpp"
#include "smoke_api/config.hpp"

namespace smoke_api::steam_http {
    bool GetHTTPResponseBodyData(
        const std::string& function_name,
        const HTTPRequestHandle hRequest,
        const uint8_t* pBodyDataBuffer,
        const uint32_t unBufferSize,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto result = original_function();

            if(config::get().log_steam_http) {
                const std::string_view buffer =
                    pBodyDataBuffer && unBufferSize
                        ? std::string_view(
                            reinterpret_cast<const char*>(pBodyDataBuffer),
                            unBufferSize
                        )
                        : "";

                LOG_INFO(
                    "{} -> handle: {}, size: {}, buffer:\n{}",
                    function_name,
                    hRequest,
                    unBufferSize,
                    buffer
                );
            }

            return result;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", __func__, e.what());
            return false;
        }
    }

    bool GetHTTPStreamingResponseBodyData(
        const std::string& function_name,
        const HTTPRequestHandle hRequest,
        const uint32_t cOffset,
        const uint8_t* pBodyDataBuffer,
        const uint32_t unBufferSize,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto result = original_function();

            if(config::get().log_steam_http) {
                const std::string_view buffer =
                    pBodyDataBuffer && unBufferSize
                        ? std::string_view(
                            reinterpret_cast<const char*>(pBodyDataBuffer),
                            unBufferSize
                        )
                        : "";

                LOG_INFO(
                    "{} -> handle: {}, offset: {}, size: {}, buffer:\n{}",
                    function_name,
                    hRequest,
                    cOffset,
                    unBufferSize,
                    buffer
                );
            }

            return result;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", __func__, e.what());
            return false;
        }
    }

    bool SetHTTPRequestRawPostBody(
        const std::string& function_name,
        const HTTPRequestHandle hRequest,
        const char* pchContentType,
        const uint8_t* pubBody,
        const uint32_t unBodyLen,
        const std::function<bool()>& original_function
    ) noexcept {
        try {
            const auto result = original_function();

            if(config::get().log_steam_http) {
                const std::string_view content_type =
                    pchContentType ? pchContentType : "smoke_api::N/A";

                const std::string_view buffer =
                    pubBody && unBodyLen
                        ? std::string_view(reinterpret_cast<const char*>(pubBody), unBodyLen)
                        : "";

                LOG_INFO(
                    "{} -> handle: {}, content-type: {}, size: {}, buffer:\n{}",
                    function_name,
                    hRequest,
                    content_type,
                    unBodyLen,
                    buffer
                );
            }

            return result;
        } catch(const std::exception& e) {
            LOG_ERROR("{} -> Error: {}", __func__, e.what());
            return false;
        }
    }
}
