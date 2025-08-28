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

## Usage

1. **Launch the app** from the Flipper Zero apps menu
2. **Press OK** to start sending BLE spam
3. **Adjust timing** using Up/Down buttons (20ms to 5 seconds)
4. **Stop** by pressing OK again

## Technical Details

### BLE Implementation
Uses the **extra_beacon API** for direct BLE control:

- `furi_hal_bt_extra_beacon_start()`: Starts custom advertising
- `furi_hal_bt_extra_beacon_set_data()`: Sets advertisement data
- `furi_hal_bt_extra_beacon_stop()`: Stops advertising

### Packet Structure
Each payload follows Apple's Continuity protocol:
- **Length byte**: Packet size (27 bytes)
- **Type byte**: Continuity message type (0x0F = Nearby Action)
- **Flags**: Behavior modifiers (0xC0 = standard, 0x40 = glitched)
- **Action**: Specific action type (0x13, 0x27, 0x20, 0x09, 0x02, 0x0B)
- **Apple Company ID**: 0x004C (little endian)

### Working Action Types
- **0x13**: AirDrop requests
- **0x27**: Apple TV connections
- **0x20**: "Join This AppleTV?" prompts
- **0x09**: iPhone setup modals
- **0x02**: Phone number transfer
- **0x0B**: Mobile backup notifications

## Troubleshooting

### No BLE Notifications
1. **Check distance**: Must be within 1 meter of target device
2. **Verify firmware**: Ensure you're using Xtreme Firmware or compatible
3. **Try different delays**: 20ms usually works best
4. **Check device state**: Some actions require unlocked devices

### Build Errors
1. **Missing SDK**: Verify Flipper Zero SDK installation
2. **Toolchain**: Check ARM GCC installation
3. **Dependencies**: Ensure `extra_beacon` requirement is met

## Safety Notes

⚠️ **Important Warnings**:

- **Legal Use**: Only test on your own devices or with explicit permission
- **Research Purpose**: This tool is for security research and testing
- **No Malicious Use**: Do not use to harass or attack others
- **Device Stability**: Some payloads may cause temporary device responses

## Credits

- **Original Implementation**: Based on work by WillyJL and ECTO-1A
- **Continuity Protocol**: Research by furiousMAC
- **Simplification**: Cleaned up and optimized by Ethan DeBruyne

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is provided for educational and research purposes only. Users are responsible for ensuring they have permission to test on target devices and for complying with applicable laws and regulations.
