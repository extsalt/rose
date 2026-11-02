# Build script for Rose syntax analyser
param (
    [switch]$Clean
)

if ($Clean) {
    Write-Host "Cleaning build artifacts..."
    Remove-Item -Force -ErrorAction SilentlyContinue rosec.exe, *.o, src/*.o
    Write-Host "Clean complete."
    exit 0
}

Write-Host "Compiling Rose Syntax Analyser..." -ForegroundColor Cyan
g++ -std=c++17 -Wall -Wextra -Iinclude src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp main.cpp -o rosec.exe

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded: rosec.exe" -ForegroundColor Green
} else {
    Write-Host "Build failed!" -ForegroundColor Red
}
