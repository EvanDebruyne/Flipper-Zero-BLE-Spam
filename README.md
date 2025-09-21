# Apple BLE Spam

A simplified Flipper Zero application that sends optimized Apple BLE Nearby Action advertisements for testing and research purposes.

## Features

- **Single Optimized Attack**: Focuses on the most effective Nearby Action spam
- **Random Action Cycling**: Automatically cycles through different working action types
- **Adjustable Timing**: Configurable advertisement intervals (20ms to 5 seconds)
- **Random MAC Addresses**: Generates unique identifiers for each session
- **Clean Codebase**: Removed unused code and simplified structure

## How It Works

This app sends **Apple Continuity protocol** BLE advertisements that trigger various popups on nearby Apple devices:

- **AirDrop requests** - "Someone is trying to share with you"
- **Apple TV connections** - "Join This AppleTV?" prompts
- **iPhone setup** - "Setup New iPhone" modals
- **Phone number transfer** - "Transfer your number" prompts
- **Mobile backup** - Various backup and sync notifications

## Project Structure

```
AppleBLEDemo/
├── apple_ble_spam.c          # Main application file
├── apple_ble_spam_icons.h    # Icon definitions
├── application.fap           # Application manifest
├── Makefile                  # Build configuration
└── README.md                 # This file
```

## Prerequisites

- Flipper Zero with Xtreme Firmware (XFW) or compatible custom firmware
- Development environment with ARM GCC toolchain
- Flipper Zero SDK
- Firmware version 7+ (API 86.0+)

## Building

### Using Makefile
```bash
make clean
make all
```

### Manual Build
```bash
# Create build directory
mkdir -p build

# Compile
gcc -o build/AppleBLEDemo.elf apple_ble_spam.c

# Convert to FAP format
objcopy -O binary build/AppleBLEDemo.elf AppleBLEDemo.fap
```

## Installation

1. Build the application using one of the methods above
2. Copy the resulting `.fap` file to your Flipper Zero's `apps` directory
3. Restart your Flipper Zero or refresh the apps menu




- **Legal Use**: Only test on your own devices or with explicit permission
- **Research Purpose**: This tool is for security research and testing


## Credits

- **Original Implementation**: Based on work by WillyJL and ECTO-1A
- **Continuity Protocol**: Research by furiousMAC
- **Simplification**: Tested and optimized by EDeBruyne

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is provided for educational and research purposes only. Users are responsible for ensuring they have permission to test on target devices and for complying with applicable laws and regulations.
