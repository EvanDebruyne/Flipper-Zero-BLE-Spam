# Makefile for Apple BLE Spam Flipper App
# This uses the Flipper Zero SDK build system

# Default target
all: build

# Build using ufbt (Flipper Zero build tool)
build:
	@echo "Building Apple BLE Spam app..."
	python -m ufbt APPDIR=. APPID=apple_ble_spam build

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	rm -rf build/
	rm -f *.fap

# Install to connected Flipper Zero
install: build
	@echo "Installing to Flipper Zero..."
	python -m ufbt APPDIR=. APPID=apple_ble_spam install

# Flash to connected Flipper Zero
flash: build
	@echo "Flashing to Flipper Zero..."
	python -m ufbt APPDIR=. APPID=apple_ble_spam flash

# Show help
help:
	@echo "Available targets:"
	@echo "  build   - Build the app (creates .fap file)"
	@echo "  clean   - Clean build artifacts"
	@echo "  install - Install to connected Flipper Zero"
	@echo "  flash   - Flash to connected Flipper Zero"
	@echo "  help    - Show this help message"

.PHONY: all build clean install flash help
