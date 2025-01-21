#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <unordered_set>
#include <esp_wifi.h> // for wifi_auth_mode_t if needed

// -------------------------------------------------------------------
// 0) GLOBAL SERVER
WebServer server(80);

// -------------------------------------------------------------------
// 1) Custom Hasher & Equality for Arduino String
struct StringHash {
  size_t operator()(const String &key) const {
    const char* str = key.c_str();
    size_t hash = 0;
    while (*str) {
      hash = 37 * hash + (unsigned char)*str++;
    }
    return hash;
  }
};

struct StringEqual {
  bool operator()(const String &a, const String &b) const {
    return a == b;
  }
};

// -------------------------------------------------------------------
// 2) Data Structures
struct Monster {
  String name;
  int level;
  int hp;
  int defense;
};

struct Player {
  String name;
  int level;
  bool hasStarter;
};

// The user’s 3-monster party
static Monster userParty[3];
static int userPartySize = 0;
static Player gPlayer = { "NoName", 1, false };

// The "wild" monsters discovered by scanning
static std::vector<Monster> gMonsters;

// BSSIDs we've encountered. We'll persist them so we skip old networks even after reboot.
static std::unordered_set<String, StringHash, StringEqual> encounteredBSSIDs;

// -------------------------------------------------------------------
// 3) Filenames
static const char* PLAYER_FILE  = "/player.json";
static const char* PARTY_FILE   = "/userparty.json";
static const char* WIGLE_FILE   = "/wigledata.csv";
static const char* BSSID_FILE   = "/bssids.json"; // store encountered BSSIDs

// -------------------------------------------------------------------
// 4) Load/Save BSSIDs in /bssids.json
bool saveEncounteredBSSIDs() {
  File f = SPIFFS.open(BSSID_FILE, "w");
  if(!f){
    Serial.println("Failed open /bssids.json for writing");
    return false;
  }
  DynamicJsonDocument doc(8192);
  JsonArray arr= doc.createNestedArray("bssids");
  for(const auto &bssid : encounteredBSSIDs){
    arr.add(bssid);
  }
  if(serializeJson(doc, f)==0){
    Serial.println("Fail write /bssids.json");
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool loadEncounteredBSSIDs() {
  if(!SPIFFS.exists(BSSID_FILE)){
    Serial.println("No /bssids.json found; starting empty.");
    return true;
  }
  File f= SPIFFS.open(BSSID_FILE,"r");
  if(!f){
    Serial.println("Failed open /bssids.json for read");
    return false;
  }
  DynamicJsonDocument doc(8192);
  DeserializationError err= deserializeJson(doc,f);
  f.close();
  if(err){
    Serial.println("Fail parse /bssids.json");
    return false;
  }
  JsonArray arr= doc["bssids"].as<JsonArray>();
  if(!arr.isNull()){
    for(const auto &elem : arr){
      encounteredBSSIDs.insert(elem.as<String>());
    }
  }
  Serial.printf("Loaded %u BSSIDs from /bssids.json\n",(unsigned)encounteredBSSIDs.size());
  return true;
}

// -------------------------------------------------------------------
// 5) Recalculate a monster's HP/defense from its level
void recalcMonsterStats(Monster &mon) {
  if(mon.level<1) mon.level=1;
  mon.hp = 30 + 5*(mon.level -1);
  mon.defense = 5 + (mon.level -1);
}

// -------------------------------------------------------------------
// 6) Load/Save Player
bool savePlayer() {
  File f = SPIFFS.open(PLAYER_FILE, "w");
  if(!f){
    Serial.println("Failed to open /player.json for write");
    return false;
  }
  DynamicJsonDocument doc(256);
  doc["name"]       = gPlayer.name;
  doc["level"]      = gPlayer.level;
  doc["hasStarter"] = gPlayer.hasStarter;
  if(serializeJson(doc, f)==0){
    Serial.println("Fail write /player.json");
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool loadPlayer() {
  if(!SPIFFS.exists(PLAYER_FILE)){
    Serial.println("No /player.json, defaults used.");
    return true;
  }
  File f= SPIFFS.open(PLAYER_FILE,"r");
  if(!f){
    Serial.println("Failed open /player.json for read");
    return false;
  }
  DynamicJsonDocument doc(256);
  DeserializationError err= deserializeJson(doc,f);
  f.close();
  if(err){
    Serial.println("Fail parse /player.json");
    return false;
  }
  gPlayer.name       = doc["name"]       | "NoName";
  gPlayer.level      = doc["level"]      | 1;
  gPlayer.hasStarter = doc["hasStarter"] | false;
  Serial.printf("Loaded Player: name=%s, level=%d, hasStarter=%d\n",
    gPlayer.name.c_str(), gPlayer.level, gPlayer.hasStarter);
  return true;
}

// -------------------------------------------------------------------
// 7) Load/Save Party
bool saveUserParty() {
  File f= SPIFFS.open(PARTY_FILE,"w");
  if(!f){
    Serial.println("Failed open /userparty.json write");
    return false;
  }
  DynamicJsonDocument doc(512);
  doc["partySize"] = userPartySize;
  JsonArray arr= doc.createNestedArray("party");
  for(int i=0; i<userPartySize; i++){
    JsonObject o= arr.createNestedObject();
    o["name"]    = userParty[i].name;
    o["level"]   = userParty[i].level;
    o["hp"]      = userParty[i].hp;
    o["defense"] = userParty[i].defense;
  }
  if(serializeJson(doc,f)==0){
    Serial.println("Fail write /userparty.json");
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool loadUserParty() {
  if(!SPIFFS.exists(PARTY_FILE)){
    Serial.println("No /userparty.json, empty party");
    userPartySize=0;
    return true;
  }
  File f= SPIFFS.open(PARTY_FILE,"r");
  if(!f){
    Serial.println("Failed open /userparty.json read");
    return false;
  }
  DynamicJsonDocument doc(512);
  DeserializationError err= deserializeJson(doc,f);
  f.close();
  if(err){
    Serial.println("Fail parse /userparty.json");
    return false;
  }
  int size= doc["partySize"] | 0;
  if(size<0) size=0;
  if(size>3) size=3;
  userPartySize= size;
  JsonArray arr= doc["party"].as<JsonArray>();
  int idx=0;
  for(JsonObject o : arr){
    if(idx>=3) break;
    userParty[idx].name    = o["name"].as<String>();
    userParty[idx].level   = o["level"].as<int>();
    userParty[idx].hp      = o["hp"]   |30;
    userParty[idx].defense = o["defense"]|5;
    idx++;
  }
  userPartySize= idx;
  Serial.printf("Loaded userParty, size=%d\n", userPartySize);
  return true;
}

// -------------------------------------------------------------------
// 8) Possibly assign a starter monster
void checkStarterMonster() {
  if(!gPlayer.hasStarter){
    if(userPartySize<3){
      Monster st;
      st.name  = "StarterPal";
      st.level = 1;
      recalcMonsterStats(st);
      userParty[userPartySize++]= st;
      saveUserParty();
    }
    gPlayer.hasStarter= true;
    savePlayer();
    Serial.println("Assigned starter monster on first run.");
  }
}

// -------------------------------------------------------------------
// 9) Original multi-column Wigle CSV
// columns: MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,Type
String encryptionTypeToString(wifi_auth_mode_t auth){
  switch(auth){
    case WIFI_AUTH_OPEN:         return "Open";
    case WIFI_AUTH_WEP:          return "WEP";
    case WIFI_AUTH_WPA_PSK:      return "WPA";
    case WIFI_AUTH_WPA2_PSK:     return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
    default: return "Unknown";
  }
}

void appendWigleRow(const String& ssid, const String& bssid, wifi_auth_mode_t auth,
                    int channel, int rssi) 
{
  File f= SPIFFS.open(WIGLE_FILE,"a");
  if(!f){
    Serial.println("Fail open wigledata.csv for append");
    return;
  }
  // If empty, write the original columns
  if(f.size()==0){
    f.println("MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,Type");
  }
  // Build row
  // If you need to replace commas in SSID, do it here
  String safeSSID= ssid;
  safeSSID.replace(",","_");
  // placeholder date/time, lat/lon
  String row = bssid + "," + safeSSID + "," + encryptionTypeToString(auth)
             + ",2023-01-01 00:00:00,"
             + String(channel) + "," + String(rssi)
             + ",0.00000,0.00000,WIFI\n";
  f.print(row);
  f.close();
}

void handleDownloadWigle(){
  if(!SPIFFS.exists(WIGLE_FILE)){
    server.send(404,"text/plain","No wigle data found");
    return;
  }
  File ff= SPIFFS.open(WIGLE_FILE,"r");
  if(!ff){
    server.send(500,"text/plain","Failed open wigledata.csv");
    return;
  }
  server.sendHeader("Content-Disposition","attachment; filename=\"wigledata.csv\"");
  server.streamFile(ff,"text/csv");
  ff.close();
}

void handleClearWigle(){
  if(SPIFFS.exists(WIGLE_FILE)){
    SPIFFS.remove(WIGLE_FILE);
    server.send(200,"text/plain","Cleared wigle data");
  } else {
    server.send(404,"text/plain","No wigle data file found");
  }
}

// -------------------------------------------------------------------
// 10) Monster Name arrays
static const char* FUN_PREFIXES[]={
  "Star","Candy","Turbo","Spark","Rainbow",
  "Mega","Fizzy","Funky","Magic","Cosmic",
  "Butter","Jolly","Mighty","Sunny","Lava"
};
static const char* FUN_SUFFIXES[]={
  "Dino","Bat","Cat","Dog","Fish",
  "Dragon","Bee","Fairy","Ghost","Bear",
  "Zard","Robot","Frog","Pup","Wizard"
};
String generateKidFriendlyName(){
  int pCount= sizeof(FUN_PREFIXES)/sizeof(FUN_PREFIXES[0]);
  int sCount= sizeof(FUN_SUFFIXES)/sizeof(FUN_SUFFIXES[0]);
  String pr= FUN_PREFIXES[random(pCount)];
  String sf= FUN_SUFFIXES[random(sCount)];
  return pr + sf;
}

// -------------------------------------------------------------------
// 11) Scan with ignoring old BSSIDs, Original wigle CSV
void scanNetworks(){
  Serial.println("Scanning networks...");
  int n= WiFi.scanNetworks(false,true);
  if(n<=0){
    Serial.println("No networks found.");
    return;
  }
  Serial.printf("Found %d networks.\n", n);

  for(int i=0; i<n;i++){
    String bssid= WiFi.BSSIDstr(i);
    if(encounteredBSSIDs.find(bssid)!= encounteredBSSIDs.end()){
      // skip duplicates
      continue;
    }
    // new BSSID => log
    encounteredBSSIDs.insert(bssid);
    saveEncounteredBSSIDs();

    wifi_auth_mode_t auth= WiFi.encryptionType(i);
    int channel= WiFi.channel(i);
    int rssi   = WiFi.RSSI(i);
    String ssid= WiFi.SSID(i);

    // original multi-col approach
    appendWigleRow(ssid,bssid,auth,channel,rssi);

    // create scaled monster
    Monster mon;
    mon.name= generateKidFriendlyName();
    int base= gPlayer.level;
    int minL= base-3; if(minL<1) minL=1;
    int maxL= base+3;
    int newLevel= random(minL, maxL+1);
    if(newLevel<1) newLevel=1;
    mon.level= newLevel;
    recalcMonsterStats(mon);

    gMonsters.push_back(mon);
  }
  Serial.println("Monsters updated after scanning.");
}

void handleScan(){
  scanNetworks();
  server.send(200,"text/plain","Scan done, monsters updated, wigle data logged.");
}

void handleMonsters(){
  DynamicJsonDocument doc(2048);
  JsonArray arr= doc["monsters"].to<JsonArray>();
  for(auto &mm: gMonsters){
    JsonObject o= arr.createNestedObject();
    o["name"]= mm.name;
    o["level"]= mm.level;
  }
  String out; 
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

// -------------------------------------------------------------------
// 12) Battle logic
struct BattleState {
  bool inProgress;
  int partyIndex;
  int wildIndex;
};

static BattleState battleState= {false,-1,-1};

void handleStartBattle(){
  if(!server.hasArg("wildIndex")|| !server.hasArg("partyIndex")){
    server.send(400,"text/plain","Need wildIndex & partyIndex");
    return;
  }
  int wIdx= server.arg("wildIndex").toInt();
  int pIdx= server.arg("partyIndex").toInt();
  if(wIdx<0|| wIdx>=(int)gMonsters.size()){
    server.send(400,"text/plain","Invalid wildIndex");
    return;
  }
  if(pIdx<0|| pIdx>=userPartySize){
    server.send(400,"text/plain","Invalid partyIndex");
    return;
  }
  battleState.inProgress= true;
  battleState.wildIndex= wIdx;
  battleState.partyIndex= pIdx;

  Monster &pm= userParty[pIdx];
  Monster &wm= gMonsters[wIdx];

  DynamicJsonDocument doc(256);
  doc["inProgress"]= true;
  doc["partyName"] = pm.name;
  doc["partyLevel"]= pm.level;
  doc["partyHP"]   = pm.hp;
  doc["wildName"]  = wm.name;
  doc["wildLevel"] = wm.level;
  doc["wildHP"]    = wm.hp;

  String out;
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

void handleBattleAction(){
  if(!battleState.inProgress){
    server.send(400,"text/plain","No battle in progress");
    return;
  }
  if(!server.hasArg("action")){
    server.send(400,"text/plain","Missing action param");
    return;
  }
  String action= server.arg("action");
  bool battleEnd= false;
  String msg;

  Monster &partyMon= userParty[battleState.partyIndex];
  Monster &wildMon = gMonsters[battleState.wildIndex];

  if(action=="attack"){
    int pDmg= random(1,6);
    int wDmg= random(1,5);
    wildMon.hp -= pDmg;
    msg += partyMon.name + " attacked for "+ String(pDmg)+" dmg. ";
    if(wildMon.hp>0){
      partyMon.hp -= wDmg;
      msg += wildMon.name + " countered for "+ String(wDmg)+" dmg. ";
    }
  } else if(action=="defend"){
    int wDmg= random(1,5)/2;
    if(wDmg<1) wDmg=1;
    partyMon.hp-= wDmg;
    msg += partyMon.name + " defended. "+ wildMon.name+" hits for "+String(wDmg)+" dmg.";
  } else if(action=="capture"){
    if(userPartySize>=3){
      msg+="Party is full! Can't capture!";
    } else {
      int chance= random(0,100);
      if(chance<30){
        msg+="Capture success! " + wildMon.name + " joined your party.";
        battleEnd= true;
        userParty[userPartySize].name   = wildMon.name;
        userParty[userPartySize].level  = wildMon.level;
        userParty[userPartySize].hp     = wildMon.hp;
        userParty[userPartySize].defense= wildMon.defense;
        userPartySize++;
        saveUserParty();
      } else {
        int wDmg= random(1,5);
        partyMon.hp-= wDmg;
        msg+="Capture failed! "+ wildMon.name+" hits for "+ String(wDmg)+ " dmg.";
      }
    }
  } else if(action=="run"){
    msg+="Ran away from battle!";
    battleEnd= true;
  } else {
    server.send(400,"text/plain","Unknown action");
    return;
  }

  // check faint
  if(wildMon.hp<=0){
    msg+=" "+ wildMon.name+" fainted! Your monster wins!";
    partyMon.level++;
    recalcMonsterStats(partyMon);
    gPlayer.level++;
    savePlayer();
    battleEnd= true;
  }
  if(partyMon.hp<=0){
    msg+=" "+ partyMon.name+" fainted! The wild monster wins!";
    battleEnd= true;
  }
  DynamicJsonDocument doc(512);
  doc["message"]= msg;
  doc["partyHP"]= partyMon.hp;
  doc["wildHP"] = wildMon.hp;
  doc["battleEnd"]= battleEnd;

  if(battleEnd){
    battleState.inProgress= false;
    // restore wild if not captured
    if(action!="capture"){
      recalcMonsterStats(wildMon);
    }
    // heal entire party
    for(int i=0; i<userPartySize; i++){
      recalcMonsterStats(userParty[i]);
    }
    saveUserParty();
  }

  String out;
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

// -------------------------------------------------------------------
// 13) Party endpoints
void handleMyParty(){
  DynamicJsonDocument doc(1024);
  doc["partySize"]= userPartySize;
  JsonArray arr= doc["party"].to<JsonArray>();
  for(int i=0;i<userPartySize;i++){
    JsonObject o= arr.createNestedObject();
    o["name"]  = userParty[i].name;
    o["level"] = userParty[i].level;
    o["hp"]    = userParty[i].hp;
    o["defense"] = userParty[i].defense;
  }
  String out;
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

void handleRemoveFromParty(){
  if(!server.hasArg("slot")){
    server.send(400,"text/plain","Missing slot");
    return;
  }
  int slot= server.arg("slot").toInt();
  if(slot<0||slot>=userPartySize){
    server.send(400,"text/plain","Invalid slot");
    return;
  }
  // cannot remove last monster
  if(userPartySize<=1){
    server.send(400,"text/plain","You cannot remove your final monster!");
    return;
  }
  for(int i=slot; i<userPartySize-1; i++){
    userParty[i]= userParty[i+1];
  }
  userPartySize--;
  saveUserParty();

  DynamicJsonDocument doc(256);
  doc["message"]= "Removed monster at slot "+String(slot);
  doc["partySize"]= userPartySize;
  JsonArray arr= doc.createNestedArray("party");
  for(int i=0;i<userPartySize;i++){
    JsonObject o= arr.createNestedObject();
    o["name"]   = userParty[i].name;
    o["level"]  = userParty[i].level;
    o["hp"]     = userParty[i].hp;
    o["defense"]= userParty[i].defense;
  }
  String out;
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

void handleSwapPartySlots(){
  if(!server.hasArg("slot1")||!server.hasArg("slot2")){
    server.send(400,"text/plain","Need slot1 & slot2");
    return;
  }
  int s1= server.arg("slot1").toInt();
  int s2= server.arg("slot2").toInt();
  if(s1<0||s1>=userPartySize|| s2<0|| s2>=userPartySize){
    server.send(400,"text/plain","Invalid slot indices");
    return;
  }
  Monster tmp= userParty[s1];
  userParty[s1]= userParty[s2];
  userParty[s2]= tmp;
  saveUserParty();

  DynamicJsonDocument doc(256);
  doc["message"]= "Swapped slots "+String(s1)+" and "+String(s2);
  doc["partySize"]= userPartySize;
  JsonArray arr= doc.createNestedArray("party");
  for(int i=0;i<userPartySize;i++){
    JsonObject o= arr.createNestedObject();
    o["name"]= userParty[i].name;
    o["level"]= userParty[i].level;
    o["hp"]= userParty[i].hp;
    o["defense"]= userParty[i].defense;
  }
  String out; 
  serializeJson(doc,out);
  server.send(200,"application/json", out);
}

// -------------------------------------------------------------------
// 14) Serve index.html, app.js
void handleRoot(){
  File ff= SPIFFS.open("/index.html","r");
  if(!ff){
    server.send(404,"text/plain","index.html not found");
    return;
  }
  server.streamFile(ff,"text/html");
  ff.close();
}

void handleAppJS(){
  File ff= SPIFFS.open("/app.js","r");
  if(!ff){
    server.send(404,"text/plain","app.js not found");
    return;
  }
  server.streamFile(ff,"text/javascript");
  ff.close();
}

void handleNotFound(){
  String path= server.uri();
  if(!path.startsWith("/")) path= "/"+ path;
  if(!SPIFFS.exists(path)){
    server.send(404,"text/plain","File not found");
    return;
  }
  File fl= SPIFFS.open(path,"r");
  if(!fl){
    server.send(500,"text/plain","Fail open file");
    return;
  }
  if(path.endsWith(".css")){
    server.streamFile(fl,"text/css");
  } else if(path.endsWith(".js")){
    server.streamFile(fl,"text/javascript");
  } else {
    server.streamFile(fl,"text/html");
  }
  fl.close();
}

// -------------------------------------------------------------------
// 15) Setup & Loop
void setup(){
  Serial.begin(115200);
  delay(500);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed.");
    return;
  }
  // load encountered BSSIDs first
  loadEncounteredBSSIDs();
  loadPlayer();
  loadUserParty();
  checkStarterMonster();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("PacketPals-AP");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  randomSeed(analogRead(0));

  server.on("/",               HTTP_GET, handleRoot);
  server.on("/app.js",         HTTP_GET, handleAppJS);

  server.on("/scan",           HTTP_GET, handleScan);
  server.on("/monsters",       HTTP_GET, handleMonsters);

  server.on("/downloadWigle",  HTTP_GET, handleDownloadWigle);
  server.on("/clearWigle",     HTTP_GET, handleClearWigle);

  server.on("/myParty",        HTTP_GET, handleMyParty);
  server.on("/removeFromParty",HTTP_GET, handleRemoveFromParty);
  server.on("/swapPartySlots", HTTP_GET, handleSwapPartySlots);

  server.on("/startBattle",    HTTP_GET, handleStartBattle);
  server.on("/battleAction",   HTTP_GET, handleBattleAction);

  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Server started. Connect to 'PacketPals-AP', open http://192.168.4.1/");
}

void loop(){
  server.handleClient();
}
