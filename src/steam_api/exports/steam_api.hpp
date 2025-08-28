#pragma once

#include "smoke_api/smoke_api.hpp"
#include "smoke_api/types.hpp"

DLL_EXPORT(bool) SteamAPI_RestartAppIfNecessary(AppId_t unOwnAppID);

DLL_EXPORT(void) SteamAPI_Shutdown();
