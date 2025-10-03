{% extends "./KoalaBox/templates/README.base.md" %}
{% block content %}

_Legit DLC Unlocker for Steamworks._

## ✨ Features

* `🔓` Emulate DLC ownership in legitimately owned games
* `🛅` Emulate Inventory item ownership
* `📄` Optional configuration

{% include "KoalaBox/templates/markdown/links.md" %}

{% include "KoalaBox/templates/markdown/intro.md" %}

{% include "KoalaBox/templates/markdown/usage.md" %}

{% include "KoalaBox/templates/markdown/install-win.md" %}

{% include "KoalaBox/templates/markdown/install-linux.md" %}

{% include "KoalaBox/templates/markdown/configuration.md" %}

## 🎓 Extra info

### 🔑 How SmokeAPI works in games with large number of DLCs

Some the games that have a large number of DLCs begin ownership verification by querying the Steamworks API for a list of all available DLCs.
Once the game receives the list, it will go over each item and check the ownership.
The issue arises from the fact that response from Steamworks SDK may max out at 64, depending on how much unowned DLCs the user has.
To alleviate this issue, SmokeAPI will make a web request to Steam API for a full list of DLCs, which works well most of the time.
Unfortunately, even the web API does not solve all of our problems, because it will return only DLCs that are available in Steam store.
This means that DLCs without a dedicated store offer, such as pre-order DLCs will be left out.
That's where the `extra_dlcs` config option comes into play.
You can specify those missing DLC IDs there, and SmokeAPI will make them available to the game.
However, this introduces the need for manual configuration, which goes against the ideals of this project.
To remedy this issue SmokeAPI will also fetch a manually maintained list of [extra DLCs] stored in a GitHub repository.
The purpose of that JSON file is to contain all the DLC IDs that are lacking a Steam store page.
This enables SmokeAPI to unlock all DLCs without any config file at all.
Feel free to report in the {forum-topic} games that have more than 64 DLCs,
_and_ have DLCs without a dedicated store page.
They will be added to the list of missing DLC IDs to facilitate config-less operation.

[extra DLCs]: https://github.com/acidicoala/public-entitlements/blob/main/steam/v2/dlc.json

{% include "KoalaBox/templates/markdown/troubleshooting.md" %}

{% include "KoalaBox/templates/markdown/building.md" %}

{% include "KoalaBox/templates/markdown/acknowledgements.md" %}{% block extra_oss_libs %}- [bshoshany/thread-pool](https://github.com/bshoshany/thread-pool)
- [batterycenter/embed](https://github.com/batterycenter/embed)

{% endblock %}