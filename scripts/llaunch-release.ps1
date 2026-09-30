#
# Build Release, then run it. Extra arguments are passed to build.ps1.
#
& (Join-Path $PSScriptRoot "build.ps1") -Configuration Release -Run @args
