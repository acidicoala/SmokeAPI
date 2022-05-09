# Usage:
# PS ..\SmokeAPI> .\build.ps1 64 Debug

Set-Location (Get-Item $PSScriptRoot)

. ./KoalaBox/build.ps1 @args

Build-Project
