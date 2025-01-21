// wifi_scanner.cpp
#include "wifi_scanner.h"

WiFiScanner::WiFiScanner() {
    // Initialize Wi-Fi in STA mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
}

std::vector<WiFiNetwork> WiFiScanner::scanNetworks() {
    std::vector<WiFiNetwork> networks;
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
        WiFiNetwork network;
        network.SSID = WiFi.SSID(i);
        network.RSSI = WiFi.RSSI(i);
        network.encryptionType = String(WiFi.encryptionType(i));
        network.BSSID = WiFi.BSSIDstr(i);
        networks.push_back(network);
    }
    return networks;
}
