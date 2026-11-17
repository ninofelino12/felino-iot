# ESP32 Wemos Lolin32 OLED OTA Project

Project configuration for **Wemos Lolin32** with an SSD1306 0.96" OLED display and Over-The-Air (OTA) firmware update support.

## Hardware Connections
- **OLED VCC** -> 3.3V
- **OLED GND** -> GND
- **OLED SDA** -> GPIO 5 (`SDA_PIN = 5`)
- **OLED SCL** -> GPIO 4 (`SCL_PIN = 4`)

## Network Configuration
- **SSID**: `Repeater-9924`
- **Password**: `nino2222`

## Building & Uploading (PlatformIO)
1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI).
2. Open this directory in PlatformIO.
3. Connect your Wemos Lolin32 via USB for the initial flash.
4. Run `pio run --target upload` to build and upload.
5. For subsequent updates, OTA is enabled (`esp32-lolin32-ota.local` or via IP address).
