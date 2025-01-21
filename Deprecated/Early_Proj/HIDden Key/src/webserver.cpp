#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wifi.h"
#include "hid.h"
#include "webserver.h"

AsyncWebServer server(80);

void setupWebServer() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS.");
        return;
    }

    server.serveStatic("/", LittleFS, "/").setDefaultFile("web_gui.html");

    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        startWiFiScan();
        request->send(200, "text/plain", "Wi-Fi scan triggered.");
    });

    server.on("/hid", HTTP_GET, [](AsyncWebServerRequest *request) {
        sendKeystrokes("Hello, World!");
        request->send(200, "text/plain", "HID payload sent.");
    });

    server.begin();
    Serial.println("Web server started.");
}
