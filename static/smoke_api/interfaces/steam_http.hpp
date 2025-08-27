#pragma once

#include "smoke_api/types.hpp"

namespace smoke_api::steam_http {
    bool GetHTTPResponseBodyData(
        const std::string& function_name,
        HTTPRequestHandle hRequest,
        const uint8_t* pBodyDataBuffer,
        uint32_t unBufferSize,
        const std::function<bool()>& original_function
    ) noexcept;

    bool GetHTTPStreamingResponseBodyData(
        const std::string& function_name,
        HTTPRequestHandle hRequest,
        uint32_t cOffset,
        const uint8_t* pBodyDataBuffer,
        uint32_t unBufferSize,
        const std::function<bool()>& original_function
    ) noexcept;

    bool SetHTTPRequestRawPostBody(
        const std::string& function_name,
        HTTPRequestHandle hRequest,
        const char* pchContentType,
        const uint8_t* pubBody,
        uint32_t unBodyLen,
        const std::function<bool()>& original_function
    ) noexcept;
}
