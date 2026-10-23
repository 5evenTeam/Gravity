@echo off
echo ============================================================
echo   Compiling Gravity Chess 4000 Engine (C++20 -O3)
echo ============================================================
g++ -O3 -std=c++20 -march=native src/bitboard.cpp src/zobrist.cpp src/board.cpp src/movegen.cpp src/evaluate.cpp src/tt.cpp src/search.cpp src/book.cpp src/uci.cpp src/main.cpp -o gravity.exe
if %ERRORLEVEL% EQU 0 (
    echo [OK] Compilation Succeeded: gravity.exe is ready!
) else (
    echo [ERROR] Compilation failed.
)
pause
