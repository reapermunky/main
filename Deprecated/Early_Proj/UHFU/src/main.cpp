#include <Arduino.h>
#include "wifi_scanner.h"
#include "bluetooth_scanner.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Instantiate modules
WiFiScanner wifiScanner;
BluetoothScanner bluetoothScanner;

// Instantiate web server on port 80
AsyncWebServer server(80);

// HTML Content for Web Interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Pen Tester Device</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    button { padding: 10px 20px; margin: 5px; }
    pre { text-align: left; display: inline-block; width: 90%; }
  </style>
</head>
<body>
  <h2>Pen Tester Device Interface</h2>
  <button onclick="scanWiFi()">Scan Wi-Fi</button>
  <button onclick="scanBluetooth()">Scan Bluetooth</button>
  <pre id="output"></pre>
  
<script>
function scanWiFi(){
  fetch('/scan_wifi').then(response => response.json()).then(data => {
    document.getElementById('output').textContent = JSON.stringify(data, null, 2);
  });
}

function scanBluetooth(){
  fetch('/scan_bluetooth').then(response => response.json()).then(data => {
    document.getElementById('output').textContent = JSON.stringify(data, null, 2);
  });
}
</script>
</body>
</html>
)rawliteral";

// Function to set up web server routes
void setupWebServer() {
    // Serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // Route to scan Wi-Fi networks
    server.on("/scan_wifi", HTTP_GET, [&](AsyncWebServerRequest *request){
        std::vector<WiFiNetwork> networks = wifiScanner.scanNetworks();
        String json = "[";
        for (size_t i = 0; i < networks.size(); i++) {
            json += "{";
            json += "\"SSID\":\"" + networks[i].SSID + "\",";
            json += "\"RSSI\":" + String(networks[i].RSSI) + ",";
            json += "\"Encryption\":\"" + networks[i].encryptionType + "\",";
            json += "\"BSSID\":\"" + networks[i].BSSID + "\"";
            json += "}";
            if (i < networks.size() - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // Route to scan Bluetooth devices
    server.on("/scan_bluetooth", HTTP_GET, [&](AsyncWebServerRequest *request){
        std::vector<BluetoothDevice> devices = bluetoothScanner.scanDevices(5);
        String json = "[";
        for (size_t i = 0; i < devices.size(); i++) {
            json += "{";
            json += "\"Name\":\"" + devices[i].name + "\",";
            json += "\"Address\":\"" + devices[i].address + "\",";
            json += "\"RSSI\":" + String(devices[i].rssi);
            json += "}";
            if (i < devices.size() - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    server.begin();
}

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Pen Tester Device...");

    // Set up Wi-Fi in AP mode
    const char* ssid = "PenTester_AP";
    const char* password = "password123"; // Set a strong password
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Initialize Web Server
    setupWebServer();
}

void loop() {
    // Main loop can remain empty as AsyncWebServer handles requests asynchronously
}
