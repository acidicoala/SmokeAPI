# SmokeAPI tools

## Steamworks Downloader

A simple tool for downloading Steamworks SDK archive from the public GitHub repository
[cdn](https://github.com/acidicoala/cdn/tree/main/valve)
and unzipping headers and binaries into the main project for subsequent processing.

## Steamworks Parser

A more sophisticated tool that parses Steamworks SDK C++ headers
in order to build an [interface lookup map](../res/interface_lookup.json),
which is used by SmokeAPI to lookup function ordinals for specific interface versions.
