// Core: ESP8266 v3.1.2 | WiFiWebChat (ChatCore) v.6.0.3
#ifndef WIFIWEBCHAT_H
#define WIFIWEBCHAT_H
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include "protocol.h"
#include "UserList.h"
#include "Config2Flash.h"

#define MAX_LOG_SIZE 1572864

class WiFiWebChat {
public:
  typedef void (*OnMsgCallback)(uint8_t* data, size_t len);
  IdentityManager identity; NetSettings net;
  WiFiWebChat() : server(80), ws(81) {}

  void begin(NetSettings defaults) {
    LittleFS.begin(); identity.begin();
    systemSalt = String(random(1000, 9999));
    net = Config2Flash::init("netSettings", defaults);
    adminLogin = Config2Flash::init("admLog", String("admin"));
    adminPass  = Config2Flash::init("admPass", String("admin"));
    WiFi.softAPConfig(IPAddress(net.ip), IPAddress(net.gw), IPAddress(net.mask));
    WiFi.softAP(net.ssid, net.pass);
    server.collectHeaders("Cookie");
    
    server.on("/login", HTTP_POST, [this](){
      String l = server.arg("l"), p = server.arg("p");
      uint16_t u = (l == adminLogin && p == adminPass) ? 666 : identity.generateUid(l, p);
      if(identity.authorize(l, u) != 0) { server.send(403, "text/plain", "ШпиЁн!"); return; }
      identity.saveUser(u, l);
      String token = sha1(String(u) + systemSalt);
      server.sendHeader("Set-Cookie", "auth=" + String(u) + ":" + token + "; Path=/; Max-Age=3600");
      server.send(200, "text/plain", String(u));
    });

    server.on("/info", [this](){
      String json = "{\"name\":\"" + String(PROJECT_NAME) + "\",\"ver\":\"" + String(PROJECT_VER/100.0) + "\"}";
      server.send(200, "application/json", json);
    });

    server.onNotFound([this]() {
      if (checkAuthSilent() == 0 && server.uri() != "/auth.html") { server.sendHeader("Location", "/auth.html"); server.send(302); return; }
      File f = LittleFS.open(server.uri() == "/" ? "/index.html" : server.uri(), "r");
      server.streamFile(f, "text/html"); f.close();
    });

    server.begin(); ws.begin();
    ws.onEvent([this](uint8_t n, WStype_t t, uint8_t* d, size_t l){ this->onWsEvent(n,t,d,l); });
  }

  void handle() { server.handleClient(); ws.loop(); }
  void setOnMessage(OnMsgCallback cb) { onMsgCb = cb; }

  void parseExternal(uint8_t* d, size_t l) {
    if (l < 7) return;
    M_Head h; memcpy(&h, d, 7);
    logMessage(d, l);
    if (h.destination_id == 0x3FFF) ws.broadcastBIN(d, l);
    else for (int i=0; i<5; i++) if (users[i].act && users[i].uid == h.destination_id) ws.sendBIN(users[i].num, d, l);
  }

private:
  ESP8266WebServer server; WebSocketsServer ws;
  struct User { uint8_t num; bool act; uint16_t uid; bool auth; } users[5]; 
  OnMsgCallback onMsgCb = nullptr;
  String adminLogin, adminPass, systemSalt;

  void logMessage(uint8_t* d, size_t l) {
    File f = LittleFS.open("/chat.bin", "a");
    if (f) { uint16_t plen = (uint16_t)l; f.write((uint8_t*)&plen, 2); f.write(d, l); f.close(); }
  }

  void sendHistory(uint8_t num, uint16_t myUid) {
    File f = LittleFS.open("/chat.bin", "r"); if(!f) return;
    while (f.available() > 2) {
      uint16_t plen; f.read((uint8_t*)&plen, 2);
      uint8_t* buf = (uint8_t*)malloc(plen); f.read(buf, plen);
      M_Head h; memcpy(&h, buf, 7);
      if (h.destination_id == 0x3FFF || h.destination_id == myUid || h.sender_id == myUid) ws.sendBIN(num, buf, plen);
      free(buf);
    }
    f.close();
    sendUserList(num);
  }

  void sendUserList(uint8_t num) {
    File f = LittleFS.open(USER_DB, "r");
    if(!f) return;
    while(f.available() >= sizeof(UserEntry)) {
      UserEntry ue; f.read((uint8_t*)&ue, sizeof(ue));
      uint8_t p[48]; 
      memset(p, 0, 48);
      M_Hello* m = (M_Hello*)p; 
      m->typeM = MSG_HELLO; 
      m->sender_id = ue.uid;
      // ИСПРАВЛЕНО v.6.0.3: Убран оператор & для массива char
      strncpy(m->sender_name, ue.name, size_name); 
      ws.sendBIN(num, p, 12 + size_name + 1);
    }
    f.close();
  }

  int checkAuthSilent() {
    if (!server.hasHeader("Cookie")) return 0;
    String c = server.header("Cookie"); int s = c.indexOf("auth="); if (s == -1) return 0;
    String v = c.substring(s+5); int sp = v.indexOf(':'), e = (v.indexOf(';')==-1)?v.length():v.indexOf(';');
    if (sp == -1) return 0;
    String uStr = v.substring(0, sp), hashStr = v.substring(sp+1, e);
    if (sha1(uStr + systemSalt) != hashStr) return 0;
    return uStr.toInt();
  }

  void onWsEvent(uint8_t n, WStype_t t, uint8_t* d, size_t l){
    if(t == WStype_BIN && l >= 3){
      int sid = -1;
      for(int i = 0; i < 5; i++) if(users[i].act && users[i].num == n) sid = i;
      if(sid == -1){ 
        for(int i = 0; i < 5; i++) if(!users[i].act){ sid = i; users[sid].num = n; users[sid].act = true; break; }
      }
      if(sid == -1) return;

      uint8_t type = d[0] & 0x0F;
      if(type == MSG_HELLO) {
        M_Hello h; memcpy(&h, d, (l < sizeof(M_Hello) ? l : sizeof(M_Hello)));
        users[sid].uid = h.sender_id; users[sid].auth = true;
        sendHistory(n, h.sender_id);
      }
      if(type == MSG_DATA && users[sid].auth) {
        M_Head h; memcpy(&h, d, 7); 
        h.sender_id = users[sid].uid; 
        memcpy(d, &h, 7); 
        logMessage(d, l);
        if(h.destination_id == 0x3FFF) ws.broadcastBIN(d, l);
        else {
          for(int i=0; i<5; i++) if(users[i].act && users[i].uid == h.destination_id) ws.sendBIN(users[i].num, d, l);
          ws.sendBIN(n, d, l); 
        }
        if(onMsgCb && (h.destination_id & group_id_mask) == group_id_mask) onMsgCb(d, l);
      }
    }
    if(t == WStype_DISCONNECTED) for(int i=0;i<5;i++) if(users[i].num == n) users[i].act = false;
  }
};
#endif
