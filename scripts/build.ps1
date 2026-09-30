#
# Configure + build DirectKeycast with CMake presets, optionally launch it.
#
#   ./scripts/build.ps1                          # Debug, auto-detected Visual Studio
#   ./scripts/build.ps1 -Configuration Release
#   ./scripts/build.ps1 -Run                     # build, then start the exe
#   ./scripts/build.ps1 -Preset vs2022 -Fresh    # force a generator, wipe the cache
#
[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [ValidateSet("auto", "vs2026", "vs2022")]
    [string]$Preset = "auto",
    [switch]$Fresh,
    [switch]$Run
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$exePath = Join-Path $buildDir "bin/$Configuration/directkeycast.exe"

$generators = @{
    "vs2026" = "Visual Studio 18 2026"
    "vs2022" = "Visual Studio 17 2022"
}

function Resolve-Preset {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio/Installer/vswhere.exe"
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw "vswhere.exe not found. Install Visual Studio 2022/2026 with the C++ desktop workload, or pass -Preset."
    }
    $version = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
    if (-not $version) {
        throw "No Visual Studio installation with MSVC x64 tools was found."
    }
    switch ([int]($version.Split(".")[0])) {
        { $_ -ge 18 } { return "vs2026" }
        17 { return "vs2022" }
        default { throw "Visual Studio $version is too old; 2022 or newer is required." }
    }
}

function Get-CachedGenerator {
    $cache = Join-Path $buildDir "CMakeCache.txt"
    if (-not (Test-Path -LiteralPath $cache)) {
        return $null
    }
    $line = Select-String -LiteralPath $cache -Pattern "^CMAKE_GENERATOR:INTERNAL=(.*)$" | Select-Object -First 1
    if ($line) {
        return $line.Matches[0].Groups[1].Value
    }
    return $null
}

Push-Location $repoRoot
try {
    if (-not (Test-Path -LiteralPath "third_party/msimeui/CMakeLists.txt")) {
        Write-Host "Initializing submodules..."
        & git submodule update --init --recursive
        if ($LASTEXITCODE -ne 0) {
            throw "git submodule update failed with exit code $LASTEXITCODE."
        }
    }

    if ($Preset -eq "auto") {
        $Preset = Resolve-Preset
    }

    # Switching Visual Studio versions needs a clean cache; do it instead of failing.
    $cachedGenerator = Get-CachedGenerator
    if ($cachedGenerator -and $cachedGenerator -ne $generators[$Preset]) {
        Write-Host "Generator changed ($cachedGenerator -> $($generators[$Preset])), reconfiguring from scratch."
        $Fresh = $true
    }

    # The linker cannot overwrite a running exe; stop copies started from this build tree.
    Get-Process -Name directkeycast -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -and $_.Path.StartsWith($buildDir, [System.StringComparison]::OrdinalIgnoreCase) } |
        ForEach-Object {
            Write-Host "Stopping running instance (pid $($_.Id))."
            Stop-Process -Id $_.Id -Force
            $_.WaitForExit(3000) | Out-Null
        }

    Write-Host "Configuring with preset '$Preset'..."
    $configureArgs = @("--preset", $Preset)
    if ($Fresh) {
        $configureArgs += "--fresh"
    }
    & cmake @configureArgs
    if ($LASTEXITCODE -ne 0) {
        throw "cmake configure failed with exit code $LASTEXITCODE."
    }

    $buildPreset = "$Preset-$($Configuration.ToLowerInvariant())"
    Write-Host "Building with preset '$buildPreset'..."
    & cmake --build --preset $buildPreset
    if ($LASTEXITCODE -ne 0) {
        throw "cmake build failed with exit code $LASTEXITCODE."
    }
    Write-Host "Built $exePath"

    if ($Run) {
        & (Join-Path $PSScriptRoot "lrun.ps1") -Configuration $Configuration
    }
}
finally {
    Pop-Location
}
