// Core: ESP8266 v3.1.2 | WebWifi v.3.9.0
#include "WiFiWebChat.h"
#include "NaRuZha.h"

WiFiWebChat chat;
NaRuZha naruzha;

void onNewMsg(uint8_t* data, size_t len) {
  naruzha.sendOutgoing(data, len);
}

void setup() {
  Serial.begin(9600); 
  chat.setOnMessage(onNewMsg);
  
  NetSettings ns;
  strncpy(ns.ssid, "ESP_G1", 31);
  strncpy(ns.pass, "guest567", 63);
  ns.ip = (uint32_t)IPAddress(192,168,34,10);
  ns.gw = (uint32_t)IPAddress(192,168,34,1);
  ns.mask = (uint32_t)IPAddress(255,255,255,0);
  ns.isAP = 1;

  chat.begin(ns);
}

void loop() {
  chat.handle();
  
  if (Serial.available() > 0) {
    uint8_t buf[256];
    size_t len = Serial.readBytes(buf, 256);
    naruzha.processIncoming(buf, len, &chat);
  }
}
