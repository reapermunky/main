#include <WiFi.h>
#include <Arduino.h>

// Function to perform a Wi-Fi scan
void startWiFiScan() {
    WiFi.mode(WIFI_IF_STA);  // Set Wi-Fi mode to Station
    WiFi.disconnect();    // Disconnect from any previous network
    delay(100);

    Serial.println("Starting Wi-Fi scan...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan completed.");
    for (int i = 0; i < n; i++) {
        Serial.printf("SSID: %s, Signal: %d dBm\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
}
