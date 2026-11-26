@echo off
setlocal
echo =================================================================
echo  EasyEDA Pro Bridge Launcher (Non-Resident / On-Demand)
echo =================================================================
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0start_easyeda_bridge.ps1"
set EXIT_CODE=%ERRORLEVEL%
echo.
echo =================================================================
echo  Bridge finished (Exit Code: %EXIT_CODE%)
echo =================================================================
pause
endlocal & exit /b %EXIT_CODE%
