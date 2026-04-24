// Core: ESP8266 v3.1.2 | WiFiWebChat v.1.8.5
#ifndef WIFIWEBCHAT_H
#define WIFIWEBCHAT_H
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <Hash.h>
#include "protocol.h"
#include "UserList.h"
#include "Config2Flash.h"

class WiFiWebChat {
public:
  typedef void (*OnMsgCallback)(uint8_t* data, size_t len, uint8_t slot);
  UserList udb; NetSettings net;
  WiFiWebChat() : server(80), ws(81) {}

  void begin(NetSettings defaults) {
    LittleFS.begin(); udb.begin();
    net = Config2Flash::init("netSettings", defaults);
    IPAddress _ip(net.ip), _gw(net.gw), _sn(net.mask);
    if (net.isAP) {
      WiFi.mode(WIFI_AP); WiFi.softAPConfig(_ip, _gw, _sn); WiFi.softAP(net.ssid, net.pass);
      dnsServer.start(53, "*", _ip);
    } else {
      WiFi.mode(WIFI_STA); WiFi.config(_ip, _gw, _sn); WiFi.begin(net.ssid, net.pass);
    }

    server.on("/login", HTTP_POST, [this](){
      String l = server.arg("l"), p = server.arg("p");
      uint16_t u = (uint16_t)strtoul(sha1(l+p).substring(0,4).c_str(), NULL, 16);
      UserEntry ue; bool ex = udb.getByUid(u, ue);
      if(ex && String(ue.name) != l) server.send(403, "text/plain", "ШпиЁн!");
      else { if(!ex) udb.add(u, l.c_str(), 0); server.send(200, "text/plain", String(u)); }
    });

    server.onNotFound([this]() {
      String h = server.hostHeader();
      if (net.isAP && h != IPAddress(net.ip).toString() && h != "esp.local") {
        server.sendHeader("Location", "http://" + IPAddress(net.ip).toString() + "/index.html", true);
        server.send(302, "text/plain", "");
      } else {
        String u = server.uri(); if (u == "/" || !LittleFS.exists(u)) u = "/index.html";
        File f = LittleFS.open(u, "r"); server.streamFile(f, "text/html"); f.close();
      }
    });

    server.begin(); ws.begin();
    ws.onEvent([this](uint8_t n, WStype_t t, uint8_t* d, size_t l){ this->onWsEvent(n,t,d,l); });
  }

  void handle() { server.handleClient(); ws.loop(); if(net.isAP) dnsServer.processNextRequest(); }
  void setOnMessage(OnMsgCallback cb) { onMsgCb = cb; }

private:
  ESP8266WebServer server; WebSocketsServer ws; DNSServer dnsServer;
  struct User { uint8_t num; bool act; uint16_t uid; bool auth; uint32_t last; } users[5]; 
  OnMsgCallback onMsgCb = nullptr;

  void onWsEvent(uint8_t n, WStype_t t, uint8_t* d, size_t l){
    if(t == WStype_BIN && l >= 3){
      M_Head* h = (M_Head*)d; int sid = -1;
      for(int i=0; i<5; i++) if(users[i].act && users[i].num == n) sid = i;
      if(sid == -1){ for(int i=0; i<5; i++) if(!users[i].act){ sid=i; users[sid].num=n; users[sid].act=true; break; } }
      if(sid != -1) {
        users[sid].last = millis();
        if(h->typeM == MSG_HELLO) users[sid].auth = true;
        if(users[sid].auth && onMsgCb) onMsgCb(d, l, sid);
      }
    } else if(t == WStype_DISCONNECTED){ for(int i=0;i<5;i++) if(users[i].num == n) users[i].act = false; }
  }
};
#endif

