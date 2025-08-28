# Building Apple BLE Spam FAP for Flipper Zero

## Prerequisites

Before you can build the .fap file, you need to install the Flipper Zero development environment:

### 1. Install Python
- Download and install Python 3.8 or higher from [python.org](https://python.org)
- Make sure to check "Add Python to PATH" during installation
- Verify installation by opening a new terminal and running: `python --version`

### 2. Install Flipper Zero SDK
Open a new terminal/command prompt and run:
```bash
pip install flipper-tools
```

### 3. Verify Installation
Check if the flipper tool is available:
```bash
flipper --version
```

## Building the FAP

### Option 1: Use the Build Scripts (Recommended)
- **Windows**: Double-click `build.bat` or run `build.ps1` in PowerShell
- **Linux/Mac**: Run `./build.sh` (if available)

### Option 2: Manual Build
1. Open a terminal in the project directory
2. Run: `flipper build`
3. The .fap file will be created in the `dist/` directory

## Installing on Flipper Zero

1. **Copy the .fap file** to your Flipper Zero's SD card
2. **Place it in**: `/apps/Bluetooth/` directory
3. **Restart your Flipper Zero** or refresh the apps menu
4. **Find the app** in: Apps → Bluetooth → Apple BLE Spam

## Troubleshooting

### Build Errors
- Make sure Python 3.8+ is installed and in PATH
- Verify flipper-tools is installed: `pip list | grep flipper`
- Check that all source files are present

### Missing Dependencies
If you get import errors, the app should work with the standard Flipper Zero firmware libraries.

### Installation Issues
- Ensure the .fap file is in the correct directory (`/apps/Bluetooth/`)
- Try refreshing the apps or restarting the device
- Check that your Flipper Zero firmware supports external apps

## What This App Does

The Apple BLE Spam app sends various types of Bluetooth Low Energy packets that can trigger popups and notifications on nearby Apple devices (within ~5 meters). It includes:

- AirDrop requests
- Proximity pairing (AirPods, etc.)
- AirPlay targets
- Handoff requests
- Tethering sources
- Nearby actions
- Custom crash packets

## Safety Note

This app is for educational and testing purposes only. Use responsibly and only on devices you own or have permission to test with.

## Support

If you encounter issues:
1. Check the [Flipper Zero documentation](https://docs.flipperzero.one/)
2. Verify your firmware version supports external apps
3. Ensure all dependencies are properly installed
