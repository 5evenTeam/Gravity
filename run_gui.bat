@echo off
title Gravity Chess 4000
echo ============================================================
echo   GRAVITY CHESS 4000 - Supercomputer AI Engine
echo   - Max ELO: 4000
echo   - Local C++ Engine (Bitboards + 28M NPS)
echo   - Internet Cloud Eval (Stockfish 19 Depth 70)
echo   - Syzygy 7-Piece Online Tablebases
echo   - Standard UCI & UDI Protocol
echo ============================================================
echo.
echo Starting Gravity Web Server on http://localhost:5000 ...
start "" http://localhost:5000
python gravity_server.py
pause
