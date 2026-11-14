/*
 * esp_wifi_rtdb
 *
 * Firmware for ESP8266 NodeMCU and ESP32.
 * - Connects to a configurable Wi-Fi (SSID/password).
 * - Talks to a configurable Firebase Realtime Database host.
 * - If the saved SSID is unavailable or connection fails, switches to AP mode
 *   with a captive portal (DNS redirect) to reconfigure SSID/password/RTDB host.
 * - Displays status and SSID on a 0.96" OLED display (I2C: SDA=GPIO14, SCL=GPIO12).
 *
 * Board selection is compile-time: build for "NodeMCU 1.0 (ESP-12E Module)"
 * or any ESP32 board; the sketch auto-detects via ESP8266/ESP32 macros.
 *
 * Dependencies:
 *   - Firebase ESP Client (mobizt)  [for RTDB]
 *   - Adafruit GFX Library & Adafruit SSD1306 [for 0.96" OLED]
 *   - Board core built-ins: WiFi, WebServer, DNSServer, EEPROM, Wire
 */

#include "config.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <DNSServer.h>
  #define WEB_SERVER ESP8266WebServer
  #define BOARD_NAME "ESP8266 NodeMCU"
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <DNSServer.h>
  #define WEB_SERVER WebServer
  #define BOARD_NAME "ESP32"
#else
  #error "Unsupported board: define ESP8266 or ESP32"
#endif

#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED settings ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA      14
#define OLED_SCL      12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- AP / portal settings ----------
static const char* AP_SSID_PREFIX = "ESP-Config-";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_GW(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);
static const byte DNS_PORT = 53;

// ---------- Globals ----------
DNSServer dnsServer;
WEB_SERVER server(80);

FirebaseData fbdo;
FirebaseAuth fbAuth;
FirebaseConfig fbConfig;

bool apMode = false;
unsigned long lastPush = 0;

// ---------- Helpers ----------
String macSuffix() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[8];
  snprintf(buf, sizeof(buf), "%02X%02X", mac[4], mac[5]);
  return String(buf);
}

String apSsid() { return String(AP_SSID_PREFIX) + macSuffix(); }

// ---------- OLED Helper ----------
void showOled(const String& title, const String& line1, const String& line2 = "", const String& line3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 9, SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

  display.setCursor(0, 14);
  display.println(line1);

  if (line2.length() > 0) {
    display.setCursor(0, 28);
    display.println(line2);
  }

  if (line3.length() > 0) {
    display.setCursor(0, 42);
    display.println(line3);
  }

  display.display();
}

// ---------- Captive portal HTML (Dynamic) ----------
String getPortalHtml() {
  String html = "<!DOCTYPE html>\n";
  html += "<html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n";
  html += "<title>ESP Config</title>\n";
  html += "<style>\n";
  html += "body{font-family:sans-serif;max-width:420px;margin:20px auto;padding:0 16px}\n";
  html += "h1{font-size:1.4rem}\n";
  html += "label{display:block;margin-top:12px;font-weight:600}\n";
  html += "input{width:100%;padding:8px;margin-top:4px;box-sizing:border-box}\n";
  html += "button{margin-top:18px;padding:10px 16px;width:100%;background:#0066cc;color:#fff;border:none;border-radius:6px;font-size:1rem}\n";
  html += ".note{font-size:.85rem;color:#555;margin-top:8px}\n";
  html += "</style></head><body>\n";
  html += "<h1>ESP Wi-Fi / RTDB Config</h1>\n";
  html += "<p class=\"note\">Board: " + String(BOARD_NAME) + "</p>\n";
  html += "<form method=\"POST\" action=\"/save\">\n";
  html += "<label>Wi-Fi SSID</label>\n";
  html += "<input name=\"ssid\" maxlength=\"32\" required value=\"" + String(cfg.ssid) + "\">\n";
  html += "<label>Wi-Fi Password</label>\n";
  html += "<input name=\"password\" type=\"password\" maxlength=\"64\" value=\"" + String(cfg.password) + "\">\n";
  html += "<label>RTDB Host (no https://)</label>\n";
  html += "<input name=\"host\" maxlength=\"128\" placeholder=\"xxx-default-rtdb.firebaseio.com\" value=\"" + String(cfg.rtdbHost) + "\">\n";
  html += "<label>RTDB Secret (legacy)</label>\n";
  html += "<input name=\"secret\" maxlength=\"64\" value=\"" + String(cfg.rtdbSecret) + "\">\n";
  html += "<button type=\"submit\">Save &amp; Reboot</button>\n";
  html += "</form>\n";
  html += "</body></html>\n";
  return html;
}

// ---------- Portal handlers ----------
void handleRoot() {
  server.send(200, "text/html", getPortalHtml());
}

void handleSave() {
  String s = server.arg("ssid");
  String p = server.arg("password");
  String h = server.arg("host");
  String sec = server.arg("secret");

  s.trim(); h.trim(); sec.trim();

  memset(cfg.ssid, 0, sizeof(cfg.ssid));
  memset(cfg.password, 0, sizeof(cfg.password));
  memset(cfg.rtdbHost, 0, sizeof(cfg.rtdbHost));
  memset(cfg.rtdbSecret, 0, sizeof(cfg.rtdbSecret));
  strncpy(cfg.ssid, s.c_str(), sizeof(cfg.ssid) - 1);
  strncpy(cfg.password, p.c_str(), sizeof(cfg.password) - 1);
  strncpy(cfg.rtdbHost, h.c_str(), sizeof(cfg.rtdbHost) - 1);
  strncpy(cfg.rtdbSecret, sec.c_str(), sizeof(cfg.rtdbSecret) - 1);
  configSave();

  showOled("Config Saved", "Rebooting...", "", "");
  server.send(200, "text/html",
    "<html><body><h2>Saved. Rebooting...</h2></body></html>");
  delay(1000);
  ESP.restart();
}

void handleNotFound() {
  server.sendHeader("Location", String("http://") + AP_IP.toString() + "/", true);
  server.send(302, "text/plain", "");
}

// ---------- Start AP mode ----------
void startApMode() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
  WiFi.softAP(apSsid().c_str());
  dnsServer.start(DNS_PORT, "*", AP_IP);

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println();
  Serial.println("=== AP MODE (config portal) ===");
  Serial.print("SSID: "); Serial.println(apSsid());
  Serial.print("IP:   "); Serial.println(WiFi.softAPIP());
  Serial.println("Open http://192.168.4.1/ in a browser");

  showOled("AP MODE", "SSID:", apSsid(), "IP: 192.168.4.1");
}

// ---------- STA connect ----------
bool connectSta() {
  if (strlen(cfg.ssid) == 0) return false;

  showOled("Connecting...", cfg.ssid, "Please wait...");

  // Check SSID availability
  int n = WiFi.scanNetworks();
  bool found = false;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == String(cfg.ssid)) { found = true; break; }
  }
  if (!found) {
    Serial.println("Configured SSID not found in scan.");
    showOled("SSID Not Found", cfg.ssid, "Entering AP mode");
    delay(1500);
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.ssid, cfg.password);

  Serial.print("Connecting to "); Serial.print(cfg.ssid);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection failed.");
    showOled("Connect Failed", cfg.ssid, "Entering AP mode");
    delay(1500);
    return false;
  }
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  showOled("WiFi Connected", "SSID:", cfg.ssid, WiFi.localIP().toString());
  return true;
}

// ---------- Firebase init ----------
void initFirebase() {
  fbConfig.host = cfg.rtdbHost;
  fbConfig.signer.tokens.legacy_token = cfg.rtdbSecret;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase RTDB initialized.");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.print("Boot: "); Serial.println(BOARD_NAME);

  // Initialize I2C for OLED with SDA=GPIO14, SCL=GPIO12
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    showOled("ESP Wi-Fi RTDB", "Booting...", BOARD_NAME);
  }

  configLoad();

  if (!configValid()) {
    Serial.println("No valid config; entering AP mode.");
    startApMode();
    return;
  }

  if (!connectSta()) {
    Serial.println("Entering AP mode for reconfiguration.");
    startApMode();
    return;
  }

  initFirebase();
}

// ---------- Loop ----------
void loop() {
  if (apMode) {
    dnsServer.processNextRequest();
    server.handleClient();
    return;
  }

  // Keep WiFi alive
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost; rebooting to re-evaluate.");
    showOled("WiFi Lost", "Rebooting...", "");
    delay(2000);
    ESP.restart();
  }

  // Push a heartbeat to RTDB every 10s
  if (millis() - lastPush > 10000) {
    lastPush = millis();
    String path = "/devices/" + macSuffix() + "/heartbeat";
    if (Firebase.RTDB.setInt(&fbdo, path.c_str(), (int)(millis() / 1000))) {
      Serial.println("RTDB push OK");
    } else {
      Serial.print("RTDB push failed: ");
      Serial.println(fbdo.errorReason());
    }
  }
}
