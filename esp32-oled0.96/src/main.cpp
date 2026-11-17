#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Wemos Lolin32 pins specified: SDA = GPIO 5, SCL = GPIO 4
#define SDA_PIN 5
#define SCL_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Repeater-9924";
const char* password = "nino2222";

void setupOLED() {
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 Lolin32"));
  display.println(F("OLED OTA Init..."));
  display.display();
  delay(1000);
}

void setupWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Connecting to WiFi:"));
  display.println(ssid);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    attempts++;
    if (attempts > 40) { // 20 seconds timeout, retry
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("WiFi Failed!"));
      display.println(F("Retrying..."));
      display.display();
      WiFi.begin(ssid, password);
      attempts = 0;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("WiFi Connected!"));
  display.print(F("IP: "));
  display.println(WiFi.localIP());
  display.println(F("OTA Ready..."));
  display.display();
  delay(2000);
}

void setupOTA() {
  ArduinoOTA.setHostname("esp32-lolin32-ota");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    Serial.println("Start updating " + type);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("OTA UPDATE START"));
    display.println(type);
    display.display();
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("OTA UPDATE DONE"));
    display.println(F("Rebooting..."));
    display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    Serial.printf("Progress: %u%%\r", percent);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("OTA Updating..."));
    display.print(percent);
    display.println(F("%"));
    
    // Draw progress bar
    int barWidth = map(percent, 0, 100, 0, 128);
    display.fillRect(0, 30, barWidth, 10, SSD1306_WHITE);
    display.display();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("OTA ERROR!"));
    if (error == OTA_AUTH_ERROR) display.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) display.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) display.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) display.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) display.println(F("End Failed"));
    display.display();
    delay(3000);
  });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  setupOLED();
  setupWiFi();
  setupOTA();
}

unsigned long lastUpdate = 0;

void loop() {
  ArduinoOTA.handle();

  // Update status screen every second
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("ESP32 Lolin32 OTA"));
    display.println(F("-----------------"));
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    display.print(F("RSSI: "));
    display.print(WiFi.RSSI());
    display.println(F(" dBm"));
    display.print(F("Uptime: "));
    display.print(millis() / 1000);
    display.println(F("s"));
    display.display();
  }
}
