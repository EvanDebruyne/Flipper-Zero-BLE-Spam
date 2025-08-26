@echo off
echo Building Apple BLE Spam App with ufbt...
echo.

REM Build the app using ufbt
ufbt APPDIR=. build

if %errorlevel% neq 0 (
    echo.
    echo Build failed! Check the error messages above.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo The .fap file should be in the build directory.
echo.
echo To install on Flipper Zero:
echo 1. Copy the .fap file to your Flipper's apps folder
echo 2. Or use qFlipper to install the app
echo.
pause
