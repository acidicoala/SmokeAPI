#include <koalabox/logger.hpp>

#include "smoke_api/interfaces/steam_http.hpp"
#include "steam_api/steam_interface.hpp"
#include "steam_api/virtuals/steam_api_virtuals.hpp"

VIRTUAL(bool) ISteamHTTP_GetHTTPResponseBodyData(
    PARAMS(
        const HTTPRequestHandle hRequest,
        const uint8_t* pBodyDataBuffer,
        const uint32_t unBufferSize
    )
) noexcept {
    return smoke_api::steam_http::GetHTTPResponseBodyData(
        __func__,
        hRequest,
        pBodyDataBuffer,
        unBufferSize,
        HOOKED_CALL_CLOSURE(
            ISteamHTTP_GetHTTPResponseBodyData,
            ARGS(hRequest, pBodyDataBuffer, unBufferSize)
        )
    );
}

VIRTUAL(bool) ISteamHTTP_GetHTTPStreamingResponseBodyData(
    PARAMS(
        const HTTPRequestHandle hRequest,
        const uint32_t cOffset,
        const uint8_t* pBodyDataBuffer,
        const uint32_t unBufferSize
    )
) noexcept {
    return smoke_api::steam_http::GetHTTPStreamingResponseBodyData(
        __func__,
        hRequest,
        cOffset,
        pBodyDataBuffer,
        unBufferSize,
        HOOKED_CALL_CLOSURE(
            ISteamHTTP_GetHTTPStreamingResponseBodyData,
            ARGS(hRequest, cOffset, pBodyDataBuffer, unBufferSize)
        )
    );
}

VIRTUAL(bool) ISteamHTTP_SetHTTPRequestRawPostBody(
    PARAMS(
        const HTTPRequestHandle hRequest,
        const char* pchContentType,
        const uint8_t* pubBody,
        const uint32_t unBodyLen
    )
) noexcept {
    return smoke_api::steam_http::SetHTTPRequestRawPostBody(
        __func__,
        hRequest,
        pchContentType,
        pubBody,
        unBodyLen,
        HOOKED_CALL_CLOSURE(
            ISteamHTTP_SetHTTPRequestRawPostBody,
            ARGS(hRequest, pchContentType, pubBody, unBodyLen)
        )
    );
}
