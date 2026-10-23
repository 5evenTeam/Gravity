@echo off
echo Compiling gravity.exe...
g++ -O3 -std=c++20 -march=native src/*.cpp -o gravity.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful.
) else (
    echo Build failed.
)
pause
