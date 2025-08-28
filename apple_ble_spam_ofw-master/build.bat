@echo off
echo Building Apple BLE Spam FAP...

REM Check if flipper tool is available
where flipper >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Flipper tool not found!
    echo Please install the Flipper Zero SDK first.
    echo.
    echo Installation steps:
    echo 1. Install Python 3.8+ from https://python.org
    echo 2. Install the Flipper Zero SDK:
    echo    pip install flipper-tools
    echo.
    pause
    exit /b 1
)

REM Build the FAP
echo Building FAP file...
flipper build

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS: FAP file built successfully!
    echo Look for apple_ble_spam.fap in the dist/ directory
    echo.
    echo To install on your Flipper Zero:
    echo 1. Copy the .fap file to your Flipper Zero's SD card
    echo 2. Place it in: /apps/Bluetooth/
    echo 3. Restart your Flipper Zero or refresh the apps
) else (
    echo.
    echo ERROR: Build failed!
    echo Check the error messages above for details.
)

pause
