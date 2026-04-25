// Core: ESP8266 v3.1.2 | IdentityManager & Common Types v.6.0.1
#ifndef USER_LIST_H
#define USER_LIST_H
#include <Arduino.h>
#include <LittleFS.h>
#include <Hash.h>

// Перенесено сюда из protocol.h для устранения ошибок видимости типа
struct __attribute__((packed)) NetSettings {
  char ssid[32];
  char pass[64];
  uint32_t ip;
  uint32_t gw;
  uint32_t mask;
  uint8_t isAP; 
};

#define USER_DB "/users.db"
const byte size_name = 15; 
const uint16_t group_id_mask = 0x28; // 0b0101000

struct UserEntry {
  uint16_t uid;
  char name[size_name + 1];
  uint32_t lastSeen;
};

class IdentityManager {
public:
  bool begin() {
    if (!LittleFS.exists(USER_DB)) { File f = LittleFS.open(USER_DB, "w"); f.close(); }
    return true;
  }

  uint16_t generateUid(String login, String pass) {
    String hash = sha1(login + pass);
    uint16_t u = (uint16_t)strtoul(hash.substring(0, 4).c_str(), NULL, 16);
    u &= 0x3FFF; // 14 бит
    u &= ~group_id_mask; // Убираем биты группы
    return u;
  }

  bool isGroup(uint16_t uid) { return (uid & group_id_mask) == group_id_mask; }

  uint8_t authorize(String l, uint16_t u) {
    if (l.equalsIgnoreCase("admin") && u != 666) return 2;
    UserEntry ue; File f = LittleFS.open(USER_DB, "r");
    while (f.available() >= sizeof(UserEntry)) {
      f.read((uint8_t*)&ue, sizeof(UserEntry));
      if (String(ue.name) == l && ue.uid != u) { f.close(); return 1; }
    }
    f.close(); return 0;
  }

  void saveUser(uint16_t uid, String login) {
    if (uid == 666 || uid == 0) return;
    UserEntry ue; if (getByUid(uid, ue)) return;
    ue.uid = uid; memset(ue.name, 0, size_name + 1);
    strncpy(ue.name, login.c_str(), size_name);
    ue.lastSeen = millis();
    File f = LittleFS.open(USER_DB, "a"); f.write((uint8_t*)&ue, sizeof(UserEntry)); f.close();
  }

  bool getByUid(uint16_t uid, UserEntry &res) {
    File f = LittleFS.open(USER_DB, "r"); if (!f) return false;
    while (f.available() >= sizeof(UserEntry)) {
      f.read((uint8_t*)&res, sizeof(UserEntry));
      if (res.uid == uid) { f.close(); return true; }
    }
    f.close(); return false;
  }
};
#endif
