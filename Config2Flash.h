// Core: ESP8266 v3.1.2 | Config2Flash v.1.4.2
#ifndef CONFIG2FLASH_H
#define CONFIG2FLASH_H
#include <Arduino.h>
#include <LittleFS.h>

class Config2Flash {
public:
  template <typename T>
  static T init(const char* name, T defaultValue) {
    String path = "/" + String(name) + ".var";
    T value;
    if (read(path.c_str(), value)) return value;
    write(path.c_str(), defaultValue);
    return defaultValue;
  }

  template <typename T>
  static bool read(const char* path, T &data) {
    if (!LittleFS.exists(path)) return false;
    File f = LittleFS.open(path, "r");
    if (!f) return false;
    size_t readLen = f.read((uint8_t*)&data, sizeof(T));
    f.close();
    return readLen == sizeof(T);
  }

  template <typename T>
  static bool write(const char* path, const T &data) {
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    size_t wrote = f.write((uint8_t*)&data, sizeof(T));
    f.close();
    return wrote == sizeof(T);
  }
};
#endif

