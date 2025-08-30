#pragma once

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/types.hpp"

using ESteamAPIInitResult = uint32_t;
using SteamErrMsg = char[1024];

DLL_EXPORT(bool) SteamAPI_Init();

DLL_EXPORT(bool) SteamAPI_InitSafe();

DLL_EXPORT(ESteamAPIInitResult) SteamAPI_InitFlat(const SteamErrMsg* pOutErrMsg);

DLL_EXPORT(ESteamAPIInitResult) SteamInternal_SteamAPI_Init(
    const char* pszInternalCheckInterfaceVersions,
    const SteamErrMsg* pOutErrMsg
);

DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(AppId_t unOwnAppID);

DLL_EXPORT(void) SteamAPI_Shutdown();
