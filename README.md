# ESP Wi-Fi / RTDB Config Firmware

Arduino firmware for **ESP8266 NodeMCU** and **ESP32** that connects to a configurable
Wi-Fi + Firebase Realtime Database, and falls back to an AP-mode captive portal when
the saved SSID is unavailable. It features a 0.96" OLED display (SSD1306 I2C) to show
boot status, SSIDs, and IP addresses.

## Requirements

- Arduino IDE (or arduino-cli) with ESP8266 / ESP32 board packages.
- Libraries:
  - `Firebase ESP Client` (mobizt) [for RTDB]
  - `Adafruit GFX Library` [for OLED graphics]
  - `Adafruit SSD1306` [for 0.96" OLED display]
- Built-ins: WiFi, WebServer, DNSServer, EEPROM, Wire.

## OLED Wiring (0.96" I2C)

- **SDA** -> GPIO 14
- **SCL** -> GPIO 12
- **VCC** -> 3.3V / 5V (per module specs)
- **GND** -> GND

## Build & Flash

1. Open `esp_wifi_rtdb/esp_wifi_rtdb.ino` in Arduino IDE.
2. Select board:
   - ESP8266: `NodeMCU 1.0 (ESP-12E Module)`
   - ESP32: any ESP32 dev board
3. Set upload speed (e.g. 115200) and port, then Upload.
4. Open Serial Monitor at 115200 baud.

## First-time use

No valid config is stored, so the board boots into AP mode:

- OLED shows AP SSID (`ESP-Config-XXXX`) and config IP (`192.168.4.1`).
- Connect and browse to `http://192.168.4.1/`
- Fill SSID, password, RTDB host (without `https://`), RTDB legacy secret
- Save → board reboots and connects.

## Reconfigure

If the saved SSID is not found or connection fails, the board re-enters AP mode
automatically. You can also erase EEPROM via `configClear()` or reflash.

## Notes

- RTDB host must be given WITHOUT scheme, e.g. `xxx-default-rtdb.firebaseio.com`.
- Wi-Fi password is stored in plaintext in EEPROM.
