#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

// AP credentials
const char* ssid = "ESP32-AP";
const char* password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  SPIFFS.format();
Serial.println("SPIFFS formatted.");
    Serial.begin(115200);
    delay(1000);  // Wait for Serial Monitor to initialize
    Serial.println("\nStarting...");

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Initialization Failed!");
    } else {
        Serial.println("SPIFFS Initialized.");
    }

    // Set up Wi-Fi as an Access Point
    Serial.println("Setting up Access Point...");
    if (WiFi.softAP(ssid, password)) {
        Serial.println("Access Point Created Successfully!");
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Failed to create Access Point.");
    }

    // Serve a simple webpage
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<h1>Hello, ESP32 Access Point!</h1>");
    });

    // Start the server
    server.begin();
    Serial.println("Web Server started");
}

void loop() {
    // Nothing needed here for now
}
