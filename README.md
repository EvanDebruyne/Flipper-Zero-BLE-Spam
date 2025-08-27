# Apple BLE Spam Demo

A Flipper Zero application that demonstrates various Apple BLE advertisement types for testing and research purposes.

## Features

- **Multiple Apple BLE Types**: AirPods, AirTags, Apple TV, iPhone setup, and more
- **Custom HCI Implementation**: Bypasses standard BLE stack for direct control
- **Adjustable Timing**: Configurable advertisement intervals
- **Random MAC Addresses**: Generates unique identifiers for each session
- **iOS 17 Crash Test**: Includes experimental crash payload

## Project Structure

```
AppleBLEDemo/
├── apple_ble_spam.c          # Main application file
├── lib/continuity/
│   ├── continuity.h          # Continuity protocol definitions
│   └── continuity.c          # Packet generation logic
├── apple_ble_spam_icons.h    # Icon definitions
├── application.fap           # Application manifest
├── Makefile                  # Build configuration
└── README.md                 # This file
```

## Prerequisites

- Flipper Zero with Xtreme Firmware (XFW) or compatible custom firmware
- Development environment with ARM GCC toolchain
- Flipper Zero SDK

## Building

### Option 1: Using Makefile
```bash
make clean
make all
```

### Option 2: Manual Build
```bash
# Create build directory
mkdir -p build

# Compile with proper includes
gcc -Ilib/continuity -o build/AppleBLEDemo.elf \
    apple_ble_spam.c lib/continuity/continuity.c

# Convert to FAP format
objcopy -O binary build/AppleBLEDemo.elf AppleBLEDemo.fap
```

## Installation

1. Build the application using one of the methods above
2. Copy the resulting `.fap` file to your Flipper Zero's `apps` directory
3. Restart your Flipper Zero or refresh the apps menu

## Usage

1. **Launch the app** from the Flipper Zero apps menu
2. **Navigate** through different BLE spam types using Left/Right buttons
3. **Select** a payload type and press OK to start advertising
4. **Adjust timing** using Up/Down buttons (20ms to 5 seconds)
5. **Stop** advertising by pressing OK again

## Available Payloads

### Nearby Actions
- **Transfer Phone Number**: Triggers "Transfer your number" popup
- **Setup New iPhone**: Shows iPhone setup modal
- **AppleTV AutoFill**: Banner notification for Apple TV
- **AppleTV Connecting**: Modal for TV connection

### Proximity Pairs
- **AirPods/AirPods Pro**: Headphone pairing popups
- **AirTag**: Find My device detection
- **Beats Headphones**: Various headphone models

### Special
- **iOS 17 Crash**: Experimental crash payload (use with caution)
- **Random Actions**: Cycles through different nearby actions
- **Random Pairs**: Cycles through different proximity pairs

## Technical Details

### BLE Implementation
This app uses **custom HCI commands** instead of the standard Flipper BLE API:

- `aci_gap_additional_beacon_start()`: Starts custom advertising
- `aci_gap_additional_beacon_set_data()`: Sets advertisement data
- `aci_gap_additional_beacon_stop()`: Stops advertising

### Memory Scanning
The app scans Flipper memory to find the correct HCI function addresses for different firmware versions.

### Packet Structure
Each payload type follows Apple's Continuity protocol:
- **Length byte**: Packet size
- **Type byte**: Continuity message type
- **Flags**: Behavior modifiers
- **Data**: Specific payload information
- **Apple Company ID**: 0x004C (little endian)

## Troubleshooting

### No BLE Notifications
1. **Check distance**: Must be within 1 meter of target device
2. **Verify firmware**: Ensure you're using Xtreme Firmware or compatible
3. **Check logs**: Look for error messages in the Flipper console
4. **Try different payloads**: Some work better than others

### Build Errors
1. **Missing includes**: Ensure `lib/continuity` directory exists
2. **Toolchain**: Verify ARM GCC is properly installed
3. **SDK**: Check Flipper Zero SDK installation

### Runtime Crashes
1. **Memory issues**: Some payloads may cause instability
2. **Firmware compatibility**: Try updating to latest XFW
3. **Reset device**: Power cycle the Flipper Zero

## Safety Notes

⚠️ **Important Warnings**:

- **iOS 17 Crash**: May cause target devices to freeze or crash
- **Legal Use**: Only test on your own devices or with explicit permission
- **Research Purpose**: This tool is for security research and testing
- **No Malicious Use**: Do not use to harass or attack others

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Credits

- **Original Implementation**: Based on work by WillyJL and ECTO-1A
- **Continuity Protocol**: Research by furiousMAC
- **Flipper Integration**: Adapted for Flipper Zero platform

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is provided for educational and research purposes only. Users are responsible for ensuring they have permission to test on target devices and for complying with applicable laws and regulations.
