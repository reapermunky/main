// wifi_scanner.h
#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

struct WiFiNetwork {
    String SSID;
    int RSSI;
    String encryptionType;
    String BSSID;
};

class WiFiScanner {
public:
    WiFiScanner();
    std::vector<WiFiNetwork> scanNetworks();
};

#endif
