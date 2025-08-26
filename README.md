 Apple BLE Spam - Flipper Zero App

A specialized Bluetooth Low Energy (BLE) spam application for the Flipper Zero, designed specifically to trigger various popup notifications on Apple devices.

##  Features

- **8 Apple-Specific Attack Vectors**: Continuously cycles through different Apple device popup triggers
- **Continuous Operation**: Automatically rotates between attack types for maximum effectiveness
- **MAC Address Rotation**: Changes MAC addresses every 2 seconds to avoid detection
- **Configurable Timing**: Adjustable attack intervals and packet counts
- **Real-time Statistics**: Live display of packets sent and current attack status

##  Attack Vectors

1. **AirDrop Spam** - Triggers "Someone is trying to share with you" popups
2. **Find My Spam** - Triggers Find My device detection notifications
3. **AirPods Spam** - Triggers AirPods pairing popups
4. **Number Transfer** - Triggers "Transfer your number" prompts
5. **Watch Pairing** - Triggers Apple Watch pairing requests
6. **TV Pairing** - Triggers Apple TV pairing popups
7. **Remote Pairing** - Triggers Apple Remote pairing requests
8. **HomePod Setup** - Triggers HomePod setup notifications



### Prerequisites
- Flipper Zero device
- qFlipper or manual SD card access
- Firmware version 7+ (API 86.0+)



### Installation
1. **Using qFlipper** (Recommended):
   - Open qFlipper
   - Connect your Flipper Zero
   - Go to Applications tab
   - Click Install and select `apple_ble_spam.fap`

2. **Manual Installation**:
   - Copy `apple_ble_spam.fap` to your Flipper's SD card
   - Place in the `apps` folder
   - Restart your Flipper Zero

##  Usage

### Controls
- **Up/Down**: Navigate between attack types
- **OK**: Start/Stop the attack
- **Back**: Exit the application
- **Long Press Up/Down**: Adjust attack interval
- **Long Press OK**: Toggle random MAC address generation

### Operation
1. Launch the app from your Flipper's Apps menu
2. Use Up/Down to select your preferred attack type
3. Press OK to start the attack
4. The app will automatically cycle through all 8 attack vectors
5. Press OK again to stop the attack



##  How It Works

The app generates BLE advertising packets that mimic various Apple device discovery signals. Each packet contains:

- **Flags**: LE General Discoverable + BR/EDR Not Supported
- **Service UUID**: Generic Access (0x1812)
- **Local Name**: Device-specific identifiers (e.g., "iPhone AirDrop", "AirPods Pro")

When Apple devices receive these packets, they interpret them as legitimate device discovery requests and display the corresponding popup notifications.
