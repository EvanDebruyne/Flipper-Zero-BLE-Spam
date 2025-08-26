# 🍎 Apple BLE Spam - Flipper Zero App

A specialized Bluetooth Low Energy (BLE) spam application for the Flipper Zero, designed specifically to trigger various popup notifications on Apple devices.

## 🎯 Features

- **8 Apple-Specific Attack Vectors**: Continuously cycles through different Apple device popup triggers
- **Continuous Operation**: Automatically rotates between attack types for maximum effectiveness
- **MAC Address Rotation**: Changes MAC addresses every 2 seconds to avoid detection
- **Configurable Timing**: Adjustable attack intervals and packet counts
- **Real-time Statistics**: Live display of packets sent and current attack status

## 🚀 Attack Vectors

1. **AirDrop Spam** - Triggers "Someone is trying to share with you" popups
2. **Find My Spam** - Triggers Find My device detection notifications
3. **AirPods Spam** - Triggers AirPods pairing popups
4. **Number Transfer** - Triggers "Transfer your number" prompts
5. **Watch Pairing** - Triggers Apple Watch pairing requests
6. **TV Pairing** - Triggers Apple TV pairing popups
7. **Remote Pairing** - Triggers Apple Remote pairing requests
8. **HomePod Setup** - Triggers HomePod setup notifications

## 📱 Target Devices

- iPhone (iOS 14+)
- iPad (iPadOS 14+)
- Mac (macOS 11+)
- Apple Watch (watchOS 7+)
- Apple TV (tvOS 14+)
- AirPods (all generations)

## 🛠️ Technical Details

- **Language**: C
- **Platform**: Flipper Zero
- **SDK Version**: 0.89.0+
- **BLE Protocol**: Bluetooth Low Energy Advertising
- **Packet Structure**: Custom BLE advertising data units (ADUs)
- **MAC Addresses**: Locally administered, randomly generated

## 🔧 Installation

### Prerequisites
- Flipper Zero device
- qFlipper or manual SD card access
- Firmware version 7+ (API 86.0+)

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/EvanDebruyne/Flipper-Zero-BLE-Spam.git
cd Flipper-Zero-BLE-Spam

# Build using ufbt
python -m ufbt APPDIR=. APPID=apple_ble_spam build
```

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

## 🎮 Usage

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

## ⚠️ Disclaimer

**This application is for educational and testing purposes only.**
- Only test on devices you own or have explicit permission to test
- Respect local laws and regulations regarding wireless communications
- Do not use for malicious purposes or harassment
- The developers are not responsible for misuse of this software

## 🔬 How It Works

The app generates BLE advertising packets that mimic various Apple device discovery signals. Each packet contains:

- **Flags**: LE General Discoverable + BR/EDR Not Supported
- **Service UUID**: Generic Access (0x1812)
- **Local Name**: Device-specific identifiers (e.g., "iPhone AirDrop", "AirPods Pro")

When Apple devices receive these packets, they interpret them as legitimate device discovery requests and display the corresponding popup notifications.

## 📊 Performance

- **Attack Interval**: 50ms (configurable 25-500ms)
- **Packets per Attack**: 100 before switching to next vector
- **MAC Rotation**: Every 2 seconds
- **Total Attack Cycle**: ~16 seconds for all 8 vectors

## 🐛 Troubleshooting

### Common Issues
1. **App won't start**: Ensure firmware is up to date
2. **No popups**: Check if target device has BLE enabled
3. **Build errors**: Verify ufbt is properly installed
4. **Installation fails**: Check Flipper Zero storage space

### Debug Mode
Enable verbose logging by setting `VERBOSE=yes` during build:
```bash
python -m ufbt APPDIR=. APPID=apple_ble_spam VERBOSE=yes build
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Flipper Zero community for SDK and tools
- Apple device specifications for BLE packet structures
- Open source BLE research and documentation

## 📞 Support

If you encounter issues or have questions:
- Open an issue on GitHub
- Check the [Flipper Zero documentation](https://docs.flipperzero.one/)
- Join the [Flipper Zero Discord](https://flipperzero.one/discord)

---

**Remember: With great power comes great responsibility. Use this tool ethically and responsibly!** 🚀
