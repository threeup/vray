param(
	[string]$Config = "Release",
	[string]$BuildDir = "build",
	[string]$Filter = "smoke"  # ctest -R filter
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSCommandPath
$BuildPath = Join-Path $RepoRoot $BuildDir

if (-not (Test-Path $BuildPath)) {
	New-Item -ItemType Directory -Path $BuildPath | Out-Null
}

Write-Host "Configuring CMake at $BuildPath" -ForegroundColor Cyan
cmake -S $RepoRoot -B $BuildPath -A x64

Write-Host "Building all tests (config=$Config)" -ForegroundColor Cyan
cmake --build $BuildPath --config $Config --target smoke_tests mesh_tests

# Prefer ctest to find the right binary per config
Push-Location $BuildPath
try {
	Write-Host "Running ctest -R $Filter -C $Config" -ForegroundColor Cyan
	ctest -R $Filter -C $Config --output-on-failure
	if ($LASTEXITCODE -ne 0) {
		throw "ctest reported failure ($LASTEXITCODE)"
	}
} finally {
	Pop-Location
}

Write-Host "Tests passed" -ForegroundColor Green