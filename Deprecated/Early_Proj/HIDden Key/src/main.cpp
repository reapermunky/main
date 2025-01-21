#undef FILE_READ
#undef FILE_WRITE
#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_TinyUSB.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>

// Create HID object
Adafruit_USBD_HID usb_hid;

// Create WebServer instance
AsyncWebServer server(80);

// Set up HID (Keyboard Emulation)
void setupHID() {
    usb_hid.begin();
}

// Send keystrokes via HID
void sendKeystrokes(const char *payload) {
    for (int i = 0; payload[i] != '\0'; i++) {
        usb_hid.keyboardPress(0, payload[i]);  // Include report_id (0) and keycode
        delay(50);
        usb_hid.keyboardRelease(0);           // Include report_id (0)
    }
}

// Perform Wi-Fi scan
void startWiFiScan() {
    WiFi.mode(WIFI_STA);  // Set Wi-Fi mode to Station
    WiFi.disconnect();    // Disconnect from any previous network
    delay(100);

    Serial.println("Starting Wi-Fi scan...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan completed.");
    for (int i = 0; i < n; i++) {
        Serial.printf("SSID: %s, Signal: %d dBm\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
}

// Set up the web server
void setupWebServer() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }

    // Serve the GUI
    server.serveStatic("/", LittleFS, "/").setDefaultFile("web_gui.html");

    // Endpoint to trigger Wi-Fi scan
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        startWiFiScan();
        request->send(200, "text/plain", "Wi-Fi scan triggered.");
    });

    // Endpoint to trigger HID payload
    server.on("/hid", HTTP_GET, [](AsyncWebServerRequest *request) {
        sendKeystrokes("Hello, World!");
        request->send(200, "text/plain", "HID payload sent.");
    });

    server.begin();
    Serial.println("Web server started");
}

// Set up OTA updates
void setupOTA() {
    ArduinoOTA.setHostname("Metro-S2");
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "Sketch" : "Filesystem";
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() { Serial.println("\nUpdate Complete"); });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA ready");
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Metro S2 Pen Tester starting...");

    setupHID();
    setupWebServer();
    setupOTA();
}

void loop() {
    ArduinoOTA.handle();
}
