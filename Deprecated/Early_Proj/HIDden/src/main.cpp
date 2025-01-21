#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    WiFi.begin("test-ssid", "test-password");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
}

void loop() {}
