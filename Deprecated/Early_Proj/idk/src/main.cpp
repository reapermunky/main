#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>

const char* apSSID = "ESP32-Pentest-Tool";
const char* apPassword = "password123";

AsyncWebServer server(80);
WiFiUDP udp;

void handleRoot(AsyncWebRequest *request) {
  String html = "<html><body><h1>ESP32 Pentest Tool</h1>";
  html += "<form action='/scan' method='get'>";
  html += "Target IP: <input type='text' name='ip'><br>";
  html += "Scan Type: ";
  html += "<input type='radio' name='scanType' value='tcp' checked>TCP ";
  html += "<input type='radio' name='scanType' value='udp'>UDP<br>";
  html += "<input type='submit' value='Scan Ports'>";
  html += "</form>";
    html += "<form action='/info' method='get'>";
  html += "<input type='submit' value='Info'>";
  html += "</form>";
  html += "</body></html>";
  request->send(200, "text/html", html);
}

void tcpScan(const char* ipStr, int startPort, int endPort) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    Serial.println("Invalid IP address");
    return;
  }
  Serial.print("Scanning TCP ports on ");
  Serial.println(ipStr);
  for (int port = startPort; port <= endPort; port++) {
    AsyncClient client;
    if (client.connect(ip, port)) {
      Serial.print("Port ");
      Serial.print(port);
      Serial.println(": Open (TCP)");
      client.close();
      delay(5);
    }
  }
}

void udpScan(const char* ipStr, int startPort, int endPort) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    Serial.println("Invalid IP address");
    return;
  }
  Serial.print("Scanning UDP ports on ");
  Serial.println(ipStr);
  for (int port = startPort; port <= endPort; port++) {
    udp.beginPacket(ip, port);
    udp.write((uint8_t)0); // Send a single null byte (important!)
    udp.endPacket();
    delay(5);
    int packetSize = udp.parsePacket();
    if (packetSize) {
      Serial.print("Port ");
      Serial.print(port);
      Serial.println(": Open/Filter (UDP)");
    }
  }
}

void handleScan(AsyncWebRequest *request) {
  if (request->hasParam("ip")) {
    AsyncWebParameter* p = request->getParam("ip");
    String targetIP = p->value();
    Serial.print("Target IP: ");
    Serial.println(targetIP);

    if (request->hasParam("scanType")) {
      String scanType = request->getParam("scanType")->value();
      if (scanType == "udp") {
        udpScan(targetIP.c_str(), 1, 1024);
      } else {
        tcpScan(targetIP.c_str(), 1, 1024);
      }
    } else {
      tcpScan(targetIP.c_str(), 1, 1024);
    }
    request->send(200, "text/plain", "Scan Complete");
  } else {
    request->send(400, "text/plain", "Missing IP parameter");
  }
}

void handleInfo(AsyncWebRequest *request) {
    String info = "IP Address: ";
    info += WiFi.softAPIP().toString();
    request->send(200, "text/plain", info);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(apSSID, apPassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/info", HTTP_GET, handleInfo);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
}