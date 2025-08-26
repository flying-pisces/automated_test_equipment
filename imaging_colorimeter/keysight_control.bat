@echo off
REM Keysight Power Meter Control Script
REM Upper-level management utility for device reboot and health monitoring

setlocal

REM Set the path to the Python script
set SCRIPT_PATH=%~dp0keysight_manager.py
set PYTHON_CMD=python

echo ============================================
echo    Keysight Power Meter Control Utility
echo ============================================
echo.

REM Check if Python script exists
if not exist "%SCRIPT_PATH%" (
    echo ERROR: keysight_manager.py not found in current directory
    echo Please ensure the script is in the same folder as this batch file
    pause
    exit /b 1
)

REM Parse command line arguments
if "%1"=="" goto :show_menu
if "%1"=="health" goto :health_check
if "%1"=="reset" goto :reset_device
if "%1"=="recover" goto :recover_device
if "%1"=="help" goto :show_help
goto :show_help

:show_menu
echo Please choose an option:
echo.
echo 1. Health Check - Check device status
echo 2. Soft Reset   - Reset via VISA commands
echo 3. Full Recovery- Attempt all recovery methods
echo 4. Help         - Show detailed usage
echo 5. Exit
echo.
set /p choice="Enter your choice (1-5): "

if "%choice%"=="1" goto :health_check
if "%choice%"=="2" goto :reset_device
if "%choice%"=="3" goto :recover_device
if "%choice%"=="4" goto :show_help
if "%choice%"=="5" goto :exit
echo Invalid choice. Please try again.
goto :show_menu

:health_check
echo.
echo Running health check...
echo.
%PYTHON_CMD% "%SCRIPT_PATH%" health
if %ERRORLEVEL%==0 (
    echo.
    echo ✓ Device is healthy and ready for use
) else (
    echo.
    echo ✗ Device has issues - consider running recovery
)
goto :end

:reset_device
echo.
echo Performing soft reset...
echo This will reset the device via VISA commands
echo.
%PYTHON_CMD% "%SCRIPT_PATH%" reset
if %ERRORLEVEL%==0 (
    echo.
    echo ✓ Reset completed successfully
    echo Device should be ready for use
) else (
    echo.
    echo ✗ Reset failed - try full recovery or manual power cycle
)
goto :end

:recover_device
echo.
echo Starting full recovery process...
echo This will try multiple recovery methods
echo.
%PYTHON_CMD% "%SCRIPT_PATH%" recover
if %ERRORLEVEL%==0 (
    echo.
    echo ✓ Recovery completed successfully
    echo Device should be ready for use
) else (
    echo.
    echo ✗ Automatic recovery failed
    echo Manual intervention required:
    echo   1. Physical power cycle of the device
    echo   2. Check network connections
    echo   3. Contact system administrator
)
goto :end

:show_help
echo.
echo USAGE:
echo   %~nx0 [command]
echo.
echo COMMANDS:
echo   health    - Check device health status
echo   reset     - Perform soft reset via VISA
echo   recover   - Attempt full recovery process
echo   help      - Show this help message
echo.
echo EXAMPLES:
echo   %~nx0           (Interactive menu)
echo   %~nx0 health    (Quick health check)
echo   %~nx0 reset     (Reset device)
echo   %~nx0 recover   (Full recovery)
echo.
echo DEVICE INFO:
echo   IP Address: 100.65.16.193
echo   VISA Address: TCPIP0::100.65.16.193::inst0::INSTR
echo   Web Interface: http://100.65.16.193/pm/index.html
echo.
goto :end

:end
if "%1"=="" (
    echo.
    echo Press any key to continue...
    pause >nul
)

:exit
endlocal