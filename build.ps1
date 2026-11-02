# Build script for Rose syntax analyser
param (
    [switch]$Clean,
    [switch]$Test
)

if ($Clean) {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    Remove-Item -Force -ErrorAction SilentlyContinue rosec.exe, rosec_test.exe, *.o, src/*.o, tests/*.o
    Write-Host "Clean complete." -ForegroundColor Green
    exit 0
}

Write-Host "Compiling Rose Syntax Analyser..." -ForegroundColor Cyan
g++ -std=c++17 -Wall -Wextra -Iinclude src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp main.cpp -o rosec.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed for rosec.exe!" -ForegroundColor Red
    exit 1
}
Write-Host "Build succeeded: rosec.exe" -ForegroundColor Green

if ($Test) {
    Write-Host "`nCompiling Rose Unit Test Runner..." -ForegroundColor Cyan
    g++ -std=c++17 -Wall -Wextra -Iinclude -Itests src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp tests/test_lexer.cpp tests/test_parser.cpp tests/test_main.cpp -o rosec_test.exe

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed for rosec_test.exe!" -ForegroundColor Red
        exit 1
    }
    Write-Host "Build succeeded: rosec_test.exe" -ForegroundColor Green

    Write-Host "`nRunning Unit Tests..." -ForegroundColor Cyan
    .\rosec_test.exe
    exit $LASTEXITCODE
}
