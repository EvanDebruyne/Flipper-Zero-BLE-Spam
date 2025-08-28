Write-Host "Building Apple BLE Spam FAP..." -ForegroundColor Green

# Check if flipper tool is available
try {
    $null = Get-Command flipper -ErrorAction Stop
} catch {
    Write-Host "ERROR: Flipper tool not found!" -ForegroundColor Red
    Write-Host "Please install the Flipper Zero SDK first." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Installation steps:" -ForegroundColor Cyan
    Write-Host "1. Install Python 3.8+ from https://python.org" -ForegroundColor White
    Write-Host "2. Install the Flipper Zero SDK:" -ForegroundColor White
    Write-Host "   pip install flipper-tools" -ForegroundColor White
    Write-Host ""
    Read-Host "Press Enter to continue"
    exit 1
}

# Build the FAP
Write-Host "Building FAP file..." -ForegroundColor Yellow
flipper build

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "SUCCESS: FAP file built successfully!" -ForegroundColor Green
    Write-Host "Look for apple_ble_spam.fap in the dist/ directory" -ForegroundColor White
    Write-Host ""
    Write-Host "To install on your Flipper Zero:" -ForegroundColor Cyan
    Write-Host "1. Copy the .fap file to your Flipper Zero's SD card" -ForegroundColor White
    Write-Host "2. Place it in: /apps/Bluetooth/" -ForegroundColor White
    Write-Host "3. Restart your Flipper Zero or refresh the apps" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    Write-Host "Check the error messages above for details." -ForegroundColor Yellow
}

Read-Host "Press Enter to continue"
