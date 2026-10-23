@echo off
title Gravity Chess
echo Starting Gravity server...
start "" http://localhost:5000
python gravity_server.py
pause
