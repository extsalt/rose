# Run Rose compiler unit test suite
Write-Host "Building and running Rose unit tests..." -ForegroundColor Cyan
& "$PSScriptRoot\build.ps1" -Test
