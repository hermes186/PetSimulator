@echo off
echo =============================================
echo   Pet Simulator - Compilation Script
echo =============================================
echo.

REM Check if g++ is available
where g++ > nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] g++ compiler not found.
    echo Please make sure MinGW or TDM-GCC is installed and added to PATH.
    echo.
    pause
    exit /b 1
)

echo [1/2] Compiling server...
g++ -std=c++11 -I./src -o PetSimulatorServer.exe src/main_server.cpp src/BattleResultCalculator.cpp -lws2_32

if %errorlevel% neq 0 (
    echo.
    echo [FAILED] Compilation failed, please check the error messages above.
    echo.
    pause
    exit /b 1
)

echo [2/2] Compilation successful!
echo.
echo =============================================
echo   Done!
echo   Run start_server.bat to start the server.
echo =============================================
echo.
pause
