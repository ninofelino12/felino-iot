# Project knowledge

This file gives Freebuff context about this project: goals, commands, conventions, and gotchas.

## What this project is

Arduino firmware for **ESP8266 NodeMCU** and **ESP32** boards that:

- Connects to a configurable Wi-Fi network (SSID + password).
- Talks to a configurable **Firebase Realtime Database (RTDB)** host (legacy secret).
- If the configured SSID is unavailable or connection fails, the board switches to **Access Point (AP) mode** with a captive portal (DNS redirect) so the user can change SSID, password, and RTDB host from a browser, then reboot.

Board selection is compile-time via `#ifdef ESP8266` / `#ifdef ESP32`.

## Key directories

- `esp_wifi_rtdb/` — the Arduino sketch (`esp_wifi_rtdb.ino`) plus `config.h`.
- `README.md` — build/flash instructions.

## Data flow

1. Boot → read `Config` struct from EEPROM.
2. If config invalid/empty → AP mode (captive portal).
3. Validate saved SSID against Wi-Fi scan; if not present or connect fails within timeout → AP mode.
4. Otherwise STA mode: connect, then init Firebase RTDB with saved host + secret; loop pushes heartbeat.
5. AP mode: serve config form at `http://192.168.4.1/`; on POST `/save` persist config and reboot.

## Commands

- Install Arduino IDE + ESP8266/ESP32 board packages.
- Libraries: `Firebase ESP Client` (mobizt), plus board core built-ins (WiFi, WebServer, DNSServer, EEPROM).
- Open `esp_wifi_rtdb/esp_wifi_rtdb.ino`, select board, upload.
- Serial monitor at 115200 baud.

## Conventions

- Conditional compilation for board differences; keep shared logic in the .ino.
- Config stored in EEPROM with a magic number for validity; size fixed at 256 bytes.
- AP SSID: `ESP-Config-XXXX` (last 4 of MAC), open network, IP `192.168.4.1`.
- RTDB host stored WITHOUT scheme (e.g. `xxx-default-rtdb.firebaseio.com`).

## Gotchas

- EEPROM must be `EEPROM.begin(256)` on both cores before read/write and `EEPROM.commit()` on ESP8266.
- Firebase ESP Client requires the host without `https://`.
- Captive portal relies on DNS server redirecting all queries to the AP IP.
- Storing Wi-Fi password in EEPROM is plaintext; acceptable for this embedded use case but not secure.
- Board-specific WebServer types (`ESP8266WebServer` vs `WebServer`) are aliased via macros.
