#!/usr/bin/env pwsh

# Test runner for vray project
# Builds the project and runs all tests

$ErrorActionPreference = "Continue"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "vray Test Suite Runner" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Build the project
Write-Host "Step 1: Building project..." -ForegroundColor Yellow
Write-Host ""

$buildStart = Get-Date
./make.ps1 2>&1 | Select-String "error C|fatal|tests.vcxproj"
$buildEnd = Get-Date
$buildDuration = ($buildEnd - $buildStart).TotalSeconds

if (-not (Test-Path ".\out\Release\tests.exe")) {
    Write-Host "Error: Build failed, tests.exe not found" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed in $($buildDuration.ToString("0.00"))s" -ForegroundColor Green
Write-Host ""

# Step 2: Run tests
Write-Host "Step 2: Running all tests (122 total)..." -ForegroundColor Yellow
Write-Host ""

$testExe = ".\out\Release\tests.exe"
$startTime = Get-Date
& $testExe
$endTime = Get-Date
$duration = ($endTime - $startTime).TotalSeconds

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Build: $($buildDuration.ToString("0.00"))s | Tests: $($duration.ToString("0.00"))s | Total: $(($buildDuration + $duration).ToString("0.00"))s" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
