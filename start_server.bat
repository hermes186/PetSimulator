@echo off

if not exist "PetSimulatorServer.exe" (
    echo ERROR: PetSimulatorServer.exe not found
    echo Please run compile.bat first
    pause
    exit /b 1
)

REM Temporary bypass proxy
set PROXY_WAS_ENABLED=0
reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable > nul 2>&1
if %errorlevel% equ 0 (
    for /f "tokens=3" %%a in ('reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable') do (
        if "%%a"=="0x1" (
            set PROXY_WAS_ENABLED=1
            reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable /t REG_DWORD /d 0 /f > nul
        )
    )
)

REM Remove system proxy env variables
set HTTP_PROXY=
set HTTPS_PROXY=
set http_proxy=
set https_proxy=

echo Starting Pet Simulator Server...
start "" "PetSimulatorServer.exe"
timeout /t 3 > nul

echo.
echo Server started!
echo Open browser: http://localhost:8080/index.html
echo.
pause

REM Restore proxy
if "%PROXY_WAS_ENABLED%"=="1" (
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable /t REG_DWORD /d 1 /f > nul
)
