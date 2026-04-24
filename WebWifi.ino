// Core: ESP8266 v3.1.2 | WebWifi v.1.8.5
#include "WiFiWebChat.h"
#include "NaRuZha.h"

WiFiWebChat chat;
NaRuZha naruzha;

void onNewMsg(uint8_t* data, size_t len, uint8_t slot) {
  naruzha.sendOutgoing(data, len);
}

void setup() {
  Serial.begin(9600); 
  chat.setOnMessage(onNewMsg);
  NetSettings ns = {"ESP_G1", "guest567", (uint32_t)IPAddress(192,168,34,10), (uint32_t)IPAddress(192,168,34,1), (uint32_t)IPAddress(255,255,255,0), 1};
  chat.begin(ns);
}

void loop() {
  chat.handle();
}

