# Achievement Tracking Feature

## Why this PR?

I'm working on [SteamEcho](https://github.com/ttmassa/steamecho), a WPF app that displays real-time achievement notifications for Steam games. To make this work, I needed a way to intercept achievement calls from games in real-time and log them to a file that my app can monitor.

Since SmokeAPI already hooks into the Steam API, adding achievement tracking here made more sense than writing a separate tool from scratch.

## What it does

Intercepts these Steam API calls and logs them to `achievements_notification.json`:
- `GetAchievement` 
- `SetAchievement` 
- `ClearAchievement` 
- `IndicateAchievementProgress` 
- `StoreStats`

Works with both virtual interface calls (`ISteamUserStats::GetAchievement`) and flat API exports (`SteamAPI_ISteamUserStats_GetAchievement`) since different games use different patterns.

The JSON file stores each achievement once with its latest state, automatically deduplicating entries:
```json
[
    {
        "event": "achievement_queried",
        "timestamp": "2025-10-23T14:41:43Z",
        "achievement": {
            "id": "FIRST_BOSS",
            "unlocked": true
        }
    }
]
```

## Configuration

Disabled by default. Enable it in `SmokeAPI.config.json`:
```json
{
  "track_achievements": true
}
```

## Files changed

**New files:**
- `static/smoke_api/interfaces/steam_user_stats.hpp/cpp` - Achievement tracking implementation
- `src/steam_api/virtuals/isteamuserstats.cpp` - Virtual interface hooks
- `src/steam_api/steam_api_flat.cpp` - Flat API export wrappers

**Modified files:**
- `CMakeLists.txt` - Build configuration
- `static/smoke_api/config.hpp` + `res/SmokeAPI.config.json` + `res/SmokeAPI.schema.json` - Config option
- `src/steam_api/steam_interfaces.cpp` - Hook registration
- `src/steam_api/virtuals/steam_api_virtuals.hpp` + `src/steam_api/virtuals/isteamclient.cpp` - Interface declarations
- `src/smoke_api/smoke_api.hpp/cpp` - Helper functions
- `README.md` - Documentation

## Testing

Tested with Tunic (553420) on Windows 11 64-bit in proxy mode. Successfully tracked 15 achievements with proper deduplication across multiple game launches.
