@echo off
REM ============================================================================
REM SETUP SCRIPT - Click đúp để chạy
REM ============================================================================

echo.
echo =============================================
echo   N-Body Gravity Simulation - Setup Script
echo =============================================
echo.

REM Chạy PowerShell script
powershell -ExecutionPolicy Bypass -File "%~dp0setup.ps1"

echo.
pause
