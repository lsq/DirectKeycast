#
# Build Debug. Extra arguments (-Fresh, -Preset vs2022, ...) are passed to build.ps1.
#
& (Join-Path $PSScriptRoot "build.ps1") -Configuration Debug @args
