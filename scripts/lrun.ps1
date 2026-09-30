#
# Run an exe that has already been built.
#
[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$exePath = Join-Path $repoRoot "build/bin/$Configuration/directkeycast.exe"

if (-not (Test-Path -LiteralPath $exePath)) {
    throw "Executable not found: $exePath (build it first with ./scripts/lcompile.ps1)"
}

Write-Host "Starting $exePath"
Start-Process -FilePath $exePath -WorkingDirectory (Split-Path -Parent $exePath)
