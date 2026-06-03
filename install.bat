@echo off

net session >nul 2>&1
if %errorLevel% == 0 (
    goto :adminTask
) else (
    echo Requesting administrative privileges...
    powershell -Command "Start-Process -FilePath '%0' -Verb RunAs"
    exit /b
)

:adminTask
cd /d "%~dp0"

cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug --install-prefix "C:/Program Files/Toaster-SDK/"
cmake --build build --config Debug
cmake --install build --config Debug