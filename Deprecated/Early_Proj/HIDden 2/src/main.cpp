/*************************************************************
  PacketPals AP + Web UI Example
  - Creates a Wi-Fi AP named "PacketPals-AP"
  - Serves a web UI from /index.html (on SPIFFS or LittleFS)
  - Provides endpoints: /scan, /monsters, /battle
  - Uses custom hasher for Arduino String in std::unordered_set
*************************************************************/

#include <WiFi.h>
#include <SPIFFS.h>          // or LittleFS if you prefer
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <unordered_set>

// 1) Custom Hasher & Equality for Arduino String
struct StringHash {
  size_t operator()(const String &key) const {
    const char* str = key.c_str();
    size_t hash = 0;
    while (*str) {
      hash = 37 * hash + (unsigned char)(*str++);
    }
    return hash;
  }
};

struct StringEqual {
  bool operator()(const String &a, const String &b) const {
    return a == b;  // Arduino String equality
  }
};

// File paths in SPIFFS
#define JSON_FILE_PATH    "/scanned_data.json"
#define MONSTER_FILE_PATH "/monsters.json"
#define MAX_PARTY_SIZE    6

// ----------------------------------------------------------
// Data Structures
// ----------------------------------------------------------
struct Monster {
  String bssid;
  String name;
  String type;
  int hp;
  int attack;
  int defense;
  int level;
  String rarity;
  String specialAbility;
};

struct BattleMonster {
  String name;
  int hp;
  int attack;
  int defense;
  int level;
  String specialAbility;
};

struct Player {
  Monster party[MAX_PARTY_SIZE];
  int partySize;
};

// ----------------------------------------------------------
// Global Variables
// ----------------------------------------------------------
Player gPlayer;                            // The player
std::vector<Monster> gWildMonsters;        // In-memory list of wild monsters
std::unordered_set<String, StringHash, StringEqual> encounteredBSSIDs; // track BSSIDs

WebServer server(80);  // The main web server

// Fun naming arrays for Packet Pals
static const char* NAME_PREFIXES[] = {
  "Packa", "Byte", "Net", "Ping", "Data",
  "Glitch", "Cypher", "Wire", "Flow", "Spark",
  "Bug", "Volt", "Wave", "Beacon", "Link"
};

static const char* NAME_SUFFIXES[] = {
  "pal", "bot", "ling", "zard", "tron", 
  "pup", "geist", "buddy", "drone"
};

// ----------------------------------------------------------
// Helper & Utility Functions
// ----------------------------------------------------------
String generatePacketPalName() {
  int prefixCount = sizeof(NAME_PREFIXES) / sizeof(NAME_PREFIXES[0]);
  int suffixCount = sizeof(NAME_SUFFIXES) / sizeof(NAME_SUFFIXES[0]);

  int pIdx = random(0, prefixCount);
  int sIdx = random(0, suffixCount);

  String prefix = NAME_PREFIXES[pIdx];
  String suffix = NAME_SUFFIXES[sIdx];
  return prefix + suffix;
}

int clampInt(int val, int minVal, int maxVal) {
  if (val < minVal) return minVal;
  if (val > maxVal) return maxVal;
  return val;
}

// Wait for single-char input with a timeout
char getUserChoice(unsigned long timeoutMs = 10000) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') continue;
      return c;
    }
    delay(50);
  }
  return '?';
}

// Arduino "String" is in a separate code base than std::string, so custom logic is needed for hashing.

// ----------------------------------------------------------
// Player & Party
// ----------------------------------------------------------
Monster& getActiveMonster() {
  // We assume gPlayer.partySize >= 1
  return gPlayer.party[0];
}

void initPlayerParty() {
  gPlayer.partySize = 1;

  Monster starter;
  starter.bssid          = "";
  starter.name           = "Startmon";
  starter.type           = "Neutral";
  starter.hp             = 120;
  starter.attack         = 10;
  starter.defense        = 30;
  starter.level          = 5;
  starter.rarity         = "Common";
  starter.specialAbility = "None";

  gPlayer.party[0] = starter;
}

// ----------------------------------------------------------
// Wi-Fi Scanning
// ----------------------------------------------------------
String encryptionTypeToString(wifi_auth_mode_t type) {
  switch (type) {
    case WIFI_AUTH_OPEN:          return "OPEN";
    case WIFI_AUTH_WEP:           return "WEP";
    case WIFI_AUTH_WPA_PSK:       return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:  return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
    default: return "UNKNOWN";
  }
}

void scanAndStoreNetworks() {
  Serial.println("Scanning for Wi-Fi networks...");
  int n = WiFi.scanNetworks(false, true);
  
  if (n <= 0) {
    Serial.println("No networks found.");
    return;
  } else {
    Serial.printf("%d networks found.\n", n);
  }

  DynamicJsonDocument doc(8192);
  JsonArray nets = doc.createNestedArray("networks");

  for (int i = 0; i < n; i++) {
    JsonObject obj = nets.createNestedObject();
    obj["ssid"]       = WiFi.SSID(i);
    obj["bssid"]      = WiFi.BSSIDstr(i);
    obj["rssi"]       = WiFi.RSSI(i);
    obj["encryption"] = encryptionTypeToString(WiFi.encryptionType(i));
  }

  File file = SPIFFS.open(JSON_FILE_PATH, "w");
  if (!file) {
    Serial.println("Failed to open file for writing scanned data.");
    return;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write scanned data to file.");
  } else {
    Serial.println("Network data stored successfully to scanned_data.json");
  }
  file.close();
}

// ----------------------------------------------------------
// Creating & Scaling Monsters
// ----------------------------------------------------------
int mapRSSIToHP(int rssi) {
  return 100 + (rssi + 100);
}

int calculateAttackFromEncryption(const String& enc) {
  if (enc == "WPA2_PSK" || enc == "WPA_WPA2_PSK") return 20;
  if (enc == "WEP") return 10;
  if (enc == "OPEN") return 5;
  return 15;
}

Monster createPacketPal(const String& bssid, int rssi, const String& enc) {
  Monster m;
  m.bssid    = bssid;
  m.name     = generatePacketPalName();
  m.type     = "Neutral";
  m.hp       = mapRSSIToHP(rssi);
  m.defense  = m.hp / 2;
  m.attack   = calculateAttackFromEncryption(enc);

  int baseLevel = clampInt(m.hp / 10, 1, 99);
  m.level = baseLevel;

  // Rarity logic
  if (enc == "OPEN") m.rarity = "Common";
  else if (enc == "WEP") m.rarity = "Uncommon";
  else if (enc == "WPA2_PSK" || enc == "WPA_WPA2_PSK") m.rarity = "Rare";
  else m.rarity = "Legendary"; // e.g. WPA2_ENTERPRISE or unknown

  // Ability logic
  if (enc == "WPA2_PSK")         { m.specialAbility = "Shield"; }
  else if (enc == "WEP")         { m.specialAbility = "Pierce"; }
  else if (enc == "OPEN")        { m.specialAbility = "None"; }
  else                           { m.specialAbility = "Invisibility"; }

  return m;
}

void scaleMonster(Monster &m, int playerLevel) {
  int minLevel = clampInt(playerLevel - 3, 1, 99);
  int maxLevel = clampInt(playerLevel + 3, 1, 99);
  int newLevel = random(minLevel, maxLevel+1);

  float scaleFactor = (float)newLevel / (float)m.level;
  m.level   = newLevel;
  m.hp      = clampInt((int)(m.hp * scaleFactor), 10, 999);
  m.defense = clampInt((int)(m.defense * scaleFactor), 1, 999);
  m.attack  = clampInt((int)(m.attack * scaleFactor), 1, 999);
}

/**
 * Reads scanned_data.json, creates packet pals for each new BSSID,
 * scales them to player's level, and stores them in gWildMonsters.
 */
void generateAndStoreMonsters() {
  File inputFile = SPIFFS.open(JSON_FILE_PATH, "r");
  if (!inputFile) {
    Serial.println("scanned_data.json not found or open error");
    return;
  }

  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, inputFile);
  inputFile.close();
  
  if (err) {
    Serial.println("Failed to parse scanned_data.json");
    return;
  }

  // Clear old data
  gWildMonsters.clear();

  Monster &active = getActiveMonster();
  int pLevel = active.level;

  JsonArray networks = doc["networks"];
  if (networks.isNull()) {
    Serial.println("No networks array in scanned_data.json");
    return;
  }

  for (JsonObject net : networks) {
    String bssid = net["bssid"].as<String>();
    String enc   = net["encryption"].as<String>();
    int rssi     = net["rssi"].as<int>();

    // Skip if we have encountered this BSSID
    if (encounteredBSSIDs.find(bssid) != encounteredBSSIDs.end()) {
      continue;
    }

    Monster m = createPacketPal(bssid, rssi, enc);
    scaleMonster(m, pLevel);

    encounteredBSSIDs.insert(bssid);
    gWildMonsters.push_back(m);
  }

  // Optional: If you want to store the newly generated monsters in a file (monsters.json)
  {
    DynamicJsonDocument monstersDoc(8192);
    JsonArray arr = monstersDoc.createNestedArray("monsters");
    for (auto &m : gWildMonsters) {
      JsonObject mob = arr.createNestedObject();
      mob["bssid"]   = m.bssid;
      mob["name"]    = m.name;
      mob["type"]    = m.type;
      mob["hp"]      = m.hp;
      mob["attack"]  = m.attack;
      mob["defense"] = m.defense;
      mob["level"]   = m.level;
      mob["rarity"]  = m.rarity;
      mob["ability"] = m.specialAbility;
    }
    File outFile = SPIFFS.open(MONSTER_FILE_PATH, "w");
    if (outFile) {
      serializeJson(monstersDoc, outFile);
      outFile.close();
      Serial.println("Monsters stored to /monsters.json");
    }
  }

  Serial.println("Monsters generated and stored in memory (and optionally /monsters.json).");
}

// ----------------------------------------------------------
// Battle System
// ----------------------------------------------------------
int performAttack(BattleMonster &attacker, BattleMonster &defender) {
  int damage = attacker.attack - (defender.defense / 4);
  if (damage < 1) damage = 1;

  int critChance = random(0, 100);
  if (critChance < 10) {
    Serial.println("A critical hit!");
    damage *= 2;
  }

  defender.hp -= damage;
  return damage;
}

String doBattle(int monsterIndex) {
  if (monsterIndex < 0 || monsterIndex >= (int)gWildMonsters.size()) {
    return "Invalid monster index!";
  }

  Monster &act = getActiveMonster();
  Monster &chosen = gWildMonsters[monsterIndex];

  // Convert to simpler BattleMonster for the fight
  BattleMonster pBM = {
    act.name, act.hp, act.attack, act.defense, act.level, act.specialAbility
  };
  BattleMonster wBM = {
    chosen.name, chosen.hp, chosen.attack, chosen.defense, chosen.level, chosen.specialAbility
  };

  bool continuing = true;
  bool m1Turn = true; // player first
  while (pBM.hp > 0 && wBM.hp > 0 && continuing) {
    BattleMonster &attacker = (m1Turn ? pBM : wBM);
    BattleMonster &defender = (m1Turn ? wBM : pBM);

    // For simplicity, let's do automatic attacks (no user prompts).
    int dmg = performAttack(attacker, defender);
    if (m1Turn) {
      Serial.printf("%s attacks %s for %d damage. %s HP: %d\n",
                    attacker.name.c_str(), defender.name.c_str(),
                    dmg, defender.name.c_str(), defender.hp);
    } else {
      Serial.printf("%s counters %s for %d damage. %s HP: %d\n",
                    attacker.name.c_str(), defender.name.c_str(),
                    dmg, defender.name.c_str(), defender.hp);
    }

    m1Turn = !m1Turn;

    if (pBM.hp <= 0 || wBM.hp <= 0) break;
  }

  // Decide outcome
  if (pBM.hp <= 0 && wBM.hp <= 0) {
    return "It's a draw! Both fainted!";
  } else if (pBM.hp <= 0) {
    return (pBM.name + " fainted! " + wBM.name + " wins!");
  } else if (wBM.hp <= 0) {
    return (wBM.name + " fainted! " + pBM.name + " wins!");
  }
  return "Battle ended unexpectedly.";
}

// ----------------------------------------------------------
// Web Handlers & Endpoints
// ----------------------------------------------------------
void handleRoot() {
  // Serve index.html from SPIFFS
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html not found in SPIFFS");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleScan() {
  // call our doScan
  scanAndStoreNetworks();
  generateAndStoreMonsters();
  server.send(200, "text/plain", "Scan + Monster Generation done");
}

void handleGetMonsters() {
  // Build JSON from gWildMonsters
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.createNestedArray("monsters");
  for (auto &m : gWildMonsters) {
    JsonObject obj = arr.createNestedObject();
    obj["name"]    = m.name;
    obj["level"]   = m.level;
    obj["hp"]      = m.hp;
    obj["attack"]  = m.attack;
    obj["defense"] = m.defense;
    obj["bssid"]   = m.bssid;
  }
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleBattleEndpoint() {
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "Missing 'index' parameter");
    return;
  }
  int idx = server.arg("index").toInt();
  String result = doBattle(idx);

  // Optionally remove the monster from gWildMonsters
  if (idx >= 0 && idx < (int)gWildMonsters.size()) {
    gWildMonsters.erase(gWildMonsters.begin() + idx);
  }

  server.send(200, "text/plain", result);
}

// ----------------------------------------------------------
// Setup & Loop
// ----------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed. Attempting format...");
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP("PacketPals-AP");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Initialize player
  initPlayerParty();

  // Set up web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/monsters", HTTP_GET, handleGetMonsters);
  server.on("/battle", HTTP_GET, handleBattleEndpoint);

  // For anything else, 404
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("Web server started. Connect to AP 'PacketPals-AP' and go to http://192.168.4.1/ in your browser.");
}

void loop() {
  server.handleClient();
}
