param(
	[string]$Config = "Release",
	[string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSCommandPath
$BuildPath = Join-Path $RepoRoot $BuildDir

if (-not (Test-Path $BuildPath)) {
	New-Item -ItemType Directory -Path $BuildPath | Out-Null
}

Write-Host "Configuring CMake at $BuildPath" -ForegroundColor Cyan
cmake -S $RepoRoot -B $BuildPath

Write-Host "Building mesh_tests (config=$Config)" -ForegroundColor Cyan
cmake --build $BuildPath --target mesh_tests --config $Config

$exeCandidates = @(
    (Join-Path $BuildPath "mesh_tests.exe"),
    (Join-Path (Join-Path $BuildPath $Config) "mesh_tests.exe")
)

$exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exe) {
	throw "mesh_tests.exe not found. Checked: $($exeCandidates -join ', ')"
}

Write-Host "Running $exe" -ForegroundColor Cyan
& $exe

if ($LASTEXITCODE -ne 0) {
	throw "mesh_tests failed with exit code $LASTEXITCODE"
}

Write-Host "mesh_tests passed" -ForegroundColor Green