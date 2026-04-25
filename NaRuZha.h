// Core: ESP8266 v3.1.2 | NaRuZha v.3.8.0 (Integrated Bridge)
#ifndef NARUZHA_H
#define NARUZHA_H
#include "protocol.h"

// Предварительное объявление, чтобы избежать циклической зависимости
class WiFiWebChat;

class NaRuZha {
public:
    // Теперь принимает ссылку на чат для проброса данных
    void processIncoming(uint8_t* data, size_t len, WiFiWebChat* chat) {
        if (!data || len < 3 || !chat) return;
        // Пробрасываем внешние данные в веб-чат
        chat->parseExternal(data, len);
        Serial.printf("[NaRuZha -> Chat] Bridged Type %d\n", ((M_Head*)data)->typeM);
    }

    void sendOutgoing(uint8_t* data, size_t len) {
        if (!data || len < 3) return;
        Serial.printf("[Chat -> NaRuZha] Out to Air: ID %d\n", ((M_Head*)data)->id_message);
        // Здесь: Serial.write(data, len) или отправка в Mesh
    }
};
#endif
