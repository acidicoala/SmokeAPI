Set-Location (Get-Item $PSScriptRoot)

. ./KoalaBox/build.ps1 @args

Build-Project
