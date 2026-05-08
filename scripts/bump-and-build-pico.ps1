# Bump GP2040_VERSION_OVERRIDE patch (e.g. v0.8.3-DIY -> v0.8.4-DIY) and build Pico firmware.
# Run from repo root:  pwsh -File scripts/bump-and-build-pico.ps1
# Optional:  -SkipBump  only reconfigure + build (no version edit)
# Optional:  $env:CMAKE_GENERATOR = "Visual Studio 17 2022"  if not using Ninja

param(
    [switch]$SkipBump
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$CMakeLists = Join-Path $RepoRoot "CMakeLists.txt"
$BuildDir = Join-Path $RepoRoot "build"

if (-not $SkipBump) {
    if (-not (Test-Path -LiteralPath $CMakeLists)) {
        Write-Error "Not found: $CMakeLists"
    }
    $text = Get-Content -LiteralPath $CMakeLists -Raw
    if ($text -notmatch 'set\(GP2040_VERSION_OVERRIDE "v(\d+)\.(\d+)\.(\d+)-DIY"') {
        Write-Error 'Could not find GP2040_VERSION_OVERRIDE "vMAJOR.MINOR.PATCH-DIY" in CMakeLists.txt'
    }
    $major = [int]$Matches[1]
    $minor = [int]$Matches[2]
    $patch = [int]$Matches[3] + 1
    $newCore = "v$major.$minor.$patch"
    $newText = $text -replace '(set\(GP2040_VERSION_OVERRIDE ")v\d+\.\d+\.\d+(-DIY" CACHE STRING)', "`$1$newCore`$2"
    if ($newText -eq $text) {
        Write-Error "Version replace failed (pattern mismatch)."
    }
    Set-Content -LiteralPath $CMakeLists -Value $newText -NoNewline
    Write-Host "Bumped version to ${newCore}-DIY"
}

if (-not (Test-Path -LiteralPath $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

$generator = $env:CMAKE_GENERATOR
if (-not $generator) {
    $generator = "Ninja"
}

Push-Location $BuildDir
try {
    $cmakeArgs = @(
        "..",
        "-G", $generator,
        "-DPICO_BOARD=pico",
        "-DPICO_PLATFORM=rp2040",
        "-UGP2040_VERSION_OVERRIDE"
    )
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & cmake --build . --parallel
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}

Write-Host "Done. UF2 / ELF are under: $BuildDir"
