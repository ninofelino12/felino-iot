# Panduan Compile dan Upload Firmware ESP Wi-Fi / RTDB + OLED

Dokumen ini menjelaskan langkah-langkah lengkap untuk melakukan *compile* (kompilasi) dan *upload* (unggah) firmware `esp_wifi_rtdb` ke board **ESP8266 NodeMCU** atau **ESP32** yang dilengkapi dengan OLED 0.96" (SSD1306 I2C).

---

## 1. Persiapan Hardware & Wiring OLED (0.96" I2C)

Hubungkan modul OLED 0.96" (128x64 / 128x32) ke ESP8266 / ESP32 dengan konfigurasi pin berikut:

| Pin OLED | Pin ESP8266 / ESP32 | Keterangan |
|----------|---------------------|------------|
| **SDA**  | **GPIO 14**         | I2C Data   |
| **SCL**  | **GPIO 12**         | I2C Clock  |
| **VCC**  | **3.3V** (atau 5V)  | Daya       |
| **GND**  | **GND**             | Ground     |

---

## 2. Persiapan Software & Library

### A. Arduino IDE (Metode GUI)
1. Install **Arduino IDE** (versi 1.8.x atau 2.x).
2. Tambahkan Board Manager URL di `File > Preferences > Additional Boards Manager URLs`:
   - **ESP8266**: `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - **ESP32**: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Buka **Board Manager** (`Tools > Board > Board Manager`), cari dan install:
   - `esp8266` (oleh ESP8266 Community)
   - `esp32` (oleh Espressif Systems)
4. Buka **Library Manager** (`Tools > Manage Libraries...`), cari dan install library berikut:
   - **`Firebase Arduino Client Library for ESP8266 and ESP32`** (oleh Mobizt)
   - **`Adafruit GFX Library`** (oleh Adafruit)
   - **`Adafruit SSD1306`** (oleh Adafruit)

---

## 3. Cara Compile & Upload via Arduino IDE

1. Buka folder proyek dan pilih file **`esp_wifi_rtdb/esp_wifi_rtdb.ino`** di Arduino IDE.
2. Pilih Board sesuai hardware yang digunakan:
   - **ESP8266**: `Tools > Board > ESP8266 Boards > NodeMCU 1.0 (ESP-12E Module)`
   - **ESP32**: `Tools > Board > ESP32 Arduino > ESP32 Dev Module` (atau board ESP32 Anda)
3. Pilih Port COM tempat board terhubung (`Tools > Port > COM...`).
4. Klik tombol **Verify / Compile** (ikon centang) untuk memeriksa error.
5. Klik tombol **Upload** (ikon panah kanan) untuk mengunggah firmware ke board.
6. Buka **Serial Monitor** (`Tools > Serial Monitor`) atur baud rate ke **115200** untuk melihat log boot dan status koneksi Wi-Fi / OLED.

---

## 4. Cara Compile & Upload via `arduino-cli` (Command Line)

Jika Anda menggunakan `arduino-cli` di terminal:

### A. Install Core & Library (jika belum)
```bash
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli core install esp32:esp32
arduino-cli lib install "Firebase Arduino Client Library for ESP8266 and ESP32"
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit SSD1306"
```

### B. Compile & Upload untuk ESP32
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32 esp_wifi_rtdb/esp_wifi_rtdb.ino

# Upload (ganti /dev/ttyUSB0 dengan port serial Anda)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 esp_wifi_rtdb/esp_wifi_rtdb.ino
```

### C. Compile & Upload untuk ESP8266 (NodeMCU)
```bash
# Compile
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 esp_wifi_rtdb/esp_wifi_rtdb.ino

# Upload (ganti /dev/ttyUSB0 dengan port serial Anda)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 esp_wifi_rtdb/esp_wifi_rtdb.ino
```

---

## 5. Penggunaan Pertama & Indikator OLED

1. Saat pertama kali dinyalakan (belum ada konfigurasi Wi-Fi tersimpan di EEPROM), ESP akan masuk ke **AP Mode**:
   - **OLED** akan menampilkan:
     - `AP MODE`
     - `SSID: ESP-Config-XXXX`
     - `IP: 192.168.4.1`
2. Hubungkan HP/Laptop ke Wi-Fi `ESP-Config-XXXX`, buka browser ke `http://192.168.4.1/`, masukkan SSID Wi-Fi rumah, password, dan kredensial Firebase RTDB, lalu klik **Save & Reboot**.
3. Setelah reboot, board akan mencoba terhubung ke Wi-Fi:
   - **OLED** menampilkan status `Connecting...` lalu `WiFi Connected` beserta SSID dan IP Address lokal.
   - Jika koneksi gagal, board otomatis kembali ke **AP Mode** untuk konfigurasi ulang.
