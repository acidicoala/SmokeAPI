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

1. Find `steam_api.dll`/`steam_api64.dll` file in game directory, and add `_o` to its name, e.g. `steam_api64_o.dll`.
2. Download the latest SmokeAPI release zip from [SmokeAPI Releases].
3. From SmokeAPI archive unpack `steam_api.dll`/`steam_api64.dll`, depending on the game bitness, and place it next to the original steam_api DLL file.

If the unlocker is not working as expected, then please fully read the [Generic Unlocker Installation Instructions] before seeking help in the support forum.

[Koaloader Releases]: https://github.com/acidicoala/Koaloader/releases/latest

[SmokeAPI Releases]: https://github.com/acidicoala/SmokeAPI/releases/latest

[Generic Unlocker Installation Instructions]: https://gist.github.com/acidicoala/2c131cb90e251f97c0c1dbeaf2c174dc

## ⚙ Configuration

SmokeAPI does not require any manual configuration. By default, it uses the most reasonable options and tries to unlock all DLCs that it can. However, there might be circumstances in which you need more custom-tailored behaviour, such as disabling certain DLCs, or selectively enabling just a few of them. In this case you can use a configuration file [SmokeAPI.config.json] that you can find here in this repository or in the release zip. To use it, simply place it next to the SmokeAPI DLL. It will be read upon each launch of a game. In the absence of the config file, default value specified below will be used.

| Option                  | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | Type             | Default |
|-------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------|:-------:|
| `$version`              | A technical field reserved for use by tools like GUI config editors. Do not modify this value.                                                                                                                                                                                                                                                                                                                                                                                                               | Integer          |   `2`   |
| `logging`               | Toggles generation of `SmokeAPI.log.log` file                                                                                                                                                                                                                                                                                                                                                                                                                                                                | Boolean          | `false` |
| `unlock_all`            | Toggles whether all DLCs should be unlocked by default                                                                                                                                                                                                                                                                                                                                                                                                                                                       | Boolean          | `true`  |
| `override`              | When `unlock_all` is `true`, this option serves as a blacklist of DLC IDs, which should remain locked. When `unlock_all` is `false`, this option serves as a whitelist of DLC IDs, which should become unlocked                                                                                                                                                                                                                                                                                              | List of Integers |  `[]`   |
| `extra_dlcs`            | When a game requests number of all DLCs from Steam and gets a response that is equal to or greater than 64, it means that we need to get extra DLCs because Steam returns maximum 64 values this way. In this case, SmokeAPI will fetch extra DLCs from several online source, such as Steam API and a [manually maintained list of DLC IDs] from GitHub. However, in some cases these sources doesn't return all possible DLCs. To address this issue, you can specify the missing DLC IDs¹ in this option. | Object           |  `{}`   |
| `auto_inject_inventory` | Toggles whether SmokeAPI should automatically inject a list of all registered inventory items, when a game queries user inventory                                                                                                                                                                                                                                                                                                                                                                            | Boolean          | `true`  |
| `extra_inventory_items` | A list of inventory items IDs¹ that will be added in addition to the automatically injected items                                                                                                                                                                                                                                                                                                                                                                                                            | List of Integers |  `[]`   |
| `koalageddon_config`    | An object that specifies patterns and offsets required for koalageddon mode. It can be used to override online config for testing or development purposes.                                                                                                                                                                                                                                                                                                                                                   | Object           | `null`  |

¹ DLC/Item IDs can be obtained from https://steamdb.info. You need to be logged in with your steam account in order to see accurate inventory item IDs.

[SmokeAPI.config.json]: res/SmokeAPI.config.json

[manually maintained list of DLC IDs]: https://github.com/acidicoala/public-entitlements/blob/main/steam/v1/dlc.json

## ℹ Extra info

### How SmokeAPI works in games with large number of DLCs

Some games that have a lot of DLCs begin ownership verification by querying the Steamworks API for a list of all available DLCs. Once the game receives the list, it will go over each item and check the ownership. The issue arises from the fact that response from Steamworks SDK may max out at 64, depending on how much unowned DLC the user has. To alleviate this issue, SmokeAPI will make a web request to Steam API for a full list of DLCs, which works well most of the time. Unfortunately, even the web API does not solve all of our problems, because it will only return DLCs that are available in Steam store. This means that DLCs without a dedicated store offer, such as pre-order DLCs will be left out. That's where the `extra_dlc_ids` config option comes into play. You can specify those missing DLC IDs there, and SmokeAPI will make them available to the game. However, this introduces the need for manual configuration, which goes against the ideals of this project. To remedy this issue SmokeAPI will also fetch [this document] stored in a GitHub repository. It contains all the DLC IDs missing from Steam store. The document is hand-crafted using data from https://steamdb.com. This enables SmokeAPI to unlock all DLCs without any config file at all. Feel free to report games that have more than 64 DLCs,
*and* have DLCs without a dedicated store page. They will be added to the list of missing DLC IDs to facilitate configless operation.

[this document]: https://github.com/acidicoala/public-entitlements/blob/main/steam/v1/dlc.json

## ✒️ TODO
- Describe how Koalageddon mode works and its config parameters
- Describe the organisation of the project

## 🏗️ Building from source

### Requirements
- Git
- CMake v3.24
- Visual Studio 2022
  - Tested on Windows 11 SDK (10.0.22621.0). Lower versions may be supported as well.

TODO: build.ps1

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
