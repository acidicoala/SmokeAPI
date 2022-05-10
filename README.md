# 🐨 SmokeAPI ♨

**Legit DLC Unlocker for Steamworks**
___

**Features**

- 🔓 Legit DLC Unlocking
- 🪝 Hook mode and Proxy mode installation
- 📝 Configless operation
- 🛅 Inventory emulation

📥 [Download the latest release](https://github.com/acidicoala/SmokeAPI/releases/latest)

💬 [Official forum topic](https://cs.rin.ru/forum/viewtopic.php?p=2597932#p2597932)

## ℹ Introduction

<details><summary>What is SmokeAPI?</summary>

SmokeAPI is a DLC unlocker for the games that are legitimately owned in your Steam account. It attempts to fool games that use Steamworks SDK into thinking that you own the desired DLCs. However, SmokeAPI does not modify the rest of the Steamworks SDK, hence features like multiplayer, achievements, etc. remain fully functional.

</details>


<details><summary>Supported versions</summary>

SmokeAPI aims to support all released SteamAPI versions. When it encountered a new, unsupported interface version, it will fall back on the latest supported version. Below is a list of supported interface versions:

- ISteamClient v6—v20. (Versions before 6 did not contain any DLC related interfaces)
- ISteamApps v2—v8. (Version 1 did not contain any DLC related functions)
- ISteamUser v12—v21. (Versions before 12 did not contain any DLC related functions)
- ISteamInventory v1—v3.

Steam inventory does not work in all games with steam inventory because of custom implementation, and online checks.
The list of games where inventory emulation has been shown to work is as follows:
- Hero Siege
- Project Winter
- Euro Truck Simulator 2
- Bloons TD 6

</details>

## 🛠 Installation Instructions

This unlocker supports 2 modes of installation: *Hook* mode and *Proxy* mode.
Try installing the unlocker in hook mode first. If it doesn't work, try installing it in proxy mode.

#### 🪝 Hook mode

1. Download the latest Koaloader release zip from [Koaloader Releases].
2. From Koaloader archive unpack `version.dll` from version-32/64, depending on the game bitness, and place it next to the game exe file.
3. Download the latest SmokeAPI release zip from [SmokeAPI Releases].
4. From SmokeAPI archive unpack `steam_api.dll`/`steam_api64.dll`, depending on the game bitness, rename it to `SmokeAPI.dll`, and place it next to the game exe file.

#### 🔀 Proxy mode

1. Find `steam_api.dll`/`steam_api64.dll` file in game directory, and add `_o` to it's name, e.g. `steam_api64_o.dll`.
2. Download the latest SmokeAPI release zip from [SmokeAPI Releases].
3. From SmokeAPI archive unpack `steam_api.dll`/`steam_api64.dll`, depending on the game bitness, and place it next to the original steam_api DLL file.

If the unlocker is not working as expected, then please fully read the [Generic Unlocker Installation Instructions] before seeking help in the support forum.

[Koaloader Releases]: https://github.com/acidicoala/Koaloader/releases/latest

[SmokeAPI Releases]: https://github.com/acidicoala/SmokeAPI/releases/latest

[Generic Unlocker Installation Instructions]: https://gist.github.com/acidicoala/2c131cb90e251f97c0c1dbeaf2c174dc

## ⚙ Configuration

SmokeAPI does not require any manual configuration. By default, it uses the most reasonable options and tries to unlock all DLCs that it can. However, there might be circumstances in which you need more custom-tailored behaviour. In this case you can use a configuration file [SmokeAPI.json] that you can find here in this repository. To use it, simply place it next to the SmokeAPI DLL. It will be read upon each launch of a game. In the absence of the config file, default value specified below will be used.

| Option                  | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | Type             | Default |
|-------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------|:-------:|
| `$version`              | A technical field reserved for future use by tools like GUI config editors                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | Integer          |   `1`   |
| `logging`               | Toggles generation of `*.log` file                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          | Boolean          | `false` |
| `hook_steamclient`      | When installed in hook mode, this option toggles between hooking steamclient(64).dll and steam_api(64).dll                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | Boolean          | `true`  |
| `unlock_all`            | Toggles whether all DLCs should be unlocked by default                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | Boolean          | `true`  |
| `override`              | When `unlock_all` is `true`, this option serves as a blacklist of DLC IDs, which should remain locked. When `unlock_all` is `false`, this option serves as a whitelist of DLC IDs, which should become unlocked                                                                                                                                                                                                                                                                                                                                                             | List of Integers |  `[]`   |
| `dlc_ids`               | When game requests list of all DLCs from Steam and the number of registered DLCs is greater than 64, Steam may not return all of them. In this case, SmokeAPI will fetch all released DLCs from Web API. In some games, however (like Monster Hunter: World), web api also doesn't return all possible DLCs. To address this issue, you can specify the missing DLC IDs¹ in this option. For some games (including MH:W), however, it is not necessary because SmokeAPI will also automatically fetch a [manually maintained list of DLC IDs] that are missing from web api | List of Integers |  `[]`   |
| `auto_inject_inventory` | Toggles whether SmokeAPI should automatically inject a list of all registered inventory items, when a game queries user inventory                                                                                                                                                                                                                                                                                                                                                                                                                                           | Boolean          | `true`  |
| `inventory_items`       | A list of inventory items IDs¹ that will be added in addition to the automatically injected items                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | List of Integers |  `[]`   |

¹ DLC/Item IDs can be obtained from https://steamdb.info. You need to be logged in with your steam account in order to see accurate inventory item IDs.

[SmokeAPI.json]: res/SmokeAPI.json
[manually maintained list of DLC IDs]: https://github.com/acidicoala/public-entitlements/blob/main/steam/v1/dlc.json

## 👋 Acknowledgements

SmokeAPI makes use of the following open source projects:

- [C++ Requests](https://github.com/libcpr/cpr)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [PolyHook 2](https://github.com/stevemk14ebr/PolyHook_2_0)
- [spdlog](https://github.com/gabime/spdlog)

## 📄 License

This software is licensed under [BSD Zero Clause  License], terms of which are available in [LICENSE.txt]

[BSD Zero Clause  License]: https://choosealicense.com/licenses/0bsd/

[LICENSE.txt]: LICENSE.txt
