#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <Update.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Wemos Lolin32 pins specified: SDA = GPIO 5, SCL = GPIO 4
#define SDA_PIN 5
#define SCL_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

const char* ssid = "Repeater-9924";
const char* password = "nino2222";

void handleRoot() {
  server.sendHeader("Connection", "close");
  
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Web Updater & System Info</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:0;padding:20px;background:#f4f4f9;color:#333;text-align:center;}";
  html += "h2{color:#007bff;} h3{margin-top:0;color:#333;}";
  html += ".card{background:#fff;padding:20px;border-radius:8px;box-shadow:0 0 10px rgba(0,0,0,0.1);display:inline-block;max-width:420px;width:100%;text-align:left;margin-bottom:20px;}";
  html += "table{width:100%;border-collapse:collapse;}";
  html += "th,td{padding:8px;border-bottom:1px solid #ddd;font-size:14px;}";
  html += "th{text-align:left;color:#555;width:45%;}";
  html += "input[type=file]{margin:15px 0;}";
  html += "input[type=submit]{background:#007bff;color:#fff;border:none;padding:10px 20px;border-radius:4px;cursor:pointer;font-size:16px;width:100%;}";
  html += "input[type=submit]:hover{background:#0056b3;}";
  html += "</style></head>";
  html += "<body>";
  html += "<h2>ESP32 Web Updater</h2>";
  
  // System Info Card
  html += "<div class='card'>";
  html += "<h3>System Info</h3>";
  html += "<table>";
  html += "<tr><th>Chip Model</th><td>" + String(ESP.getChipModel()) + " (Rev " + String(ESP.getChipRevision()) + ")</td></tr>";
  html += "<tr><th>CPU Frequency</th><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
  html += "<tr><th>MAC Address</th><td>" + String(WiFi.macAddress()) + "</td></tr>";
  html += "<tr><th>Free Heap</th><td>" + String(ESP.getFreeHeap() / 1024) + " KB / " + String(ESP.getHeapSize() / 1024) + " KB</td></tr>";
  html += "<tr><th>Flash Size</th><td>" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB</td></tr>";
  html += "<tr><th>Sketch Size</th><td>" + String(ESP.getSketchSize() / 1024) + " KB</td></tr>";
  html += "<tr><th>Uptime</th><td>" + String(millis() / 1000) + " s</td></tr>";
  html += "</table>";
  html += "</div><br>";

  // Firmware Update Card
  html += "<div class='card'>";
  html += "<h3>Firmware Update</h3>";
  html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
  html += "<p style='font-size:14px;margin-top:0;'>Select compiled <b>.bin</b> firmware file</p>";
  html += "<input type='file' name='update'><br>";
  html += "<input type='submit' value='Update Firmware'>";
  html += "</form>";
  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

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
  display.println(F("SysInfo & OTA..."));
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
  display.println(F("Web & OTA Ready..."));
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

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", Update.hasError() ? "FAIL" : "<script>alert('Update Success! Rebooting...');window.location='/';</script>");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("WEB OTA START"));
      display.println(upload.filename.c_str());
      display.display();

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      } else {
        int percent = (Update.progress() * 100) / Update.size();
        Serial.printf("Progress: %d%%\n", percent);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Web OTA Updating..."));
        display.print(percent);
        display.println(F("%"));
        
        int barWidth = map(percent, 0, 100, 0, 128);
        display.fillRect(0, 30, barWidth, 10, SSD1306_WHITE);
        display.display();
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Web OTA SUCCESS"));
        display.println(F("Rebooting..."));
        display.display();
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);
  setupOLED();
  setupWiFi();
  setupOTA();
  setupWebServer();
}

unsigned long lastUpdate = 0;

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // Update status screen every second
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("ESP32 Lolin32 WebOTA"));
    display.println(F("-------------------"));
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
