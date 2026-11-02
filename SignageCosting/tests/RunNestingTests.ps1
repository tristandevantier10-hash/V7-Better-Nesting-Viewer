# Run from an x64 Visual Studio Developer PowerShell.
param([string]$DependencyRoot)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Run this script from an x64 Visual Studio Developer PowerShell.'
}
if (-not $DependencyRoot) {
    $candidates = @('vcpkg_installed/x64-windows', 'vcpkg_installed/x64-windows/x64-windows', 'vcpkg/installed/x64-windows')
    foreach ($candidate in $candidates) {
        $path = Join-Path $repo $candidate
        if ((Test-Path "$path/include/SDL2/SDL.h") -and (Test-Path "$path/lib/SDL2.lib")) { $DependencyRoot = $path; break }
    }
}
if (-not $DependencyRoot) { throw 'SDL dependencies not found. Supply -DependencyRoot with an installed x64-windows directory.' }
$DependencyRoot = (Resolve-Path -LiteralPath $DependencyRoot).Path
$build = Join-Path $repo 'x64/NestingTests'
New-Item -ItemType Directory -Force -Path $build | Out-Null
$previousPath = $env:PATH
$env:PATH = "$DependencyRoot/bin;$env:PATH"
function Test-Suite([string]$Name, [string[]]$Sources, [switch]$UI) {
    $exe = Join-Path $build "$Name.exe"
    $options = @('/nologo', '/std:c++17', '/utf-8', '/EHsc', '/O2', "/Fo$build/", "/Fe$exe")
    $files = @("SignageCosting/tests/$Name.cpp") + @($Sources | ForEach-Object { "SignageCosting/$_" })
    $libraries = @('user32.lib')
    if ($UI) {
        $options += @('/I', "$DependencyRoot/include")
        $libraries += @('/link', "/LIBPATH:$DependencyRoot/lib", 'SDL2.lib', 'SDL2_ttf.lib', 'SDL2_image.lib', 'lunasvg.lib')
    }
    & cl.exe @options @files @libraries
    if ($LASTEXITCODE -ne 0) { throw "$Name failed to compile." }
    Push-Location (Join-Path $repo 'SignageCosting')
    try {
        & $exe
        if ($LASTEXITCODE -ne 0) { throw "$Name failed." }
    } finally { Pop-Location }
}
Push-Location $repo
try {
    $core = @('NestingGeometry.cpp', 'NestingCore.cpp')
    $service = $core + @('NestingService.cpp')
    $ui = @('NestingGeometry.cpp', 'Renderer.cpp', 'ShapeRenderer.cpp', 'FontManager.cpp', 'TextRenderer.cpp', 'UIElement.cpp')
    Test-Suite 'NestingGeometryTests' $core
    Test-Suite 'NestingCoreTests' ($service + @('NestingManager.cpp'))
    Test-Suite 'NestingServiceTests' $service
    Test-Suite 'DxfImportTests' ($service + @('DxfImport.cpp'))
    Test-Suite 'NestingOutlineEditorTests' $ui -UI
    Test-Suite 'NestingPreviewTests' ($ui + @('NestingRender.cpp')) -UI
} finally {
    Pop-Location
    $env:PATH = $previousPath
}
