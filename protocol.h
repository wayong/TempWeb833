// Core: ESP8266 v2.7.4 | v.6.0.1 | ТАБУ: ПРОТОКОЛ (БЕЗ СЕТЕВЫХ НАСТРОЕК)
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <Arduino.h>

// Константы размеров
const byte size_user_id = 14;         
const byte size_message_id = 12;      
// size_name теперь берется из UserList.h

const char PROJECT_NAME[] = "TABU_MONOLITH";
const uint16_t PROJECT_VER = 601; 

enum TypeMessage : uint8_t { 
  MSG_DATA = 1, MSG_COMPOUND, MSG_ONLINE, MSG_CNG_STATUS, MSG_CONFIRMATION, MSG_HELLO, MSG_CONFIG = 9 
};

enum FlagMessage : uint8_t { 
  MSG_SEND = 1, MSG_RECEIVED, MSG_DELIVERED, MSG_REDELIVERED, MSG_READ 
};

enum TypeData : uint8_t { DT_TEXT = 1, DT_AUDIO, DT_IMAGE };

// Базовый заголовок (3 байта)
struct __attribute__((packed)) MainHeaderMessage {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;
};

// Заголовок с данными (7 байт)
struct __attribute__((packed)) M_Head {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;
  uint16_t destination_id : 14; 
  uint16_t sender_id : 14;      
  uint8_t rez : 4;              
};

// Составное сообщение (10 байт)
struct __attribute__((packed)) M_Head_Part {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;
  uint16_t destination_id : 14; 
  uint16_t sender_id : 14;      
  uint8_t count_frame : 4;
  uint8_t num_frame : 4;
  uint8_t rez : 4;                        
  uint16_t id_orign_message : 12;   
};

// Изменение статуса (5 байт)
struct __attribute__((packed)) M_Status {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;
  uint16_t sender_id : 14;
  uint8_t rez : 2;              
};

// Знакомство (12 байт + имя)
struct __attribute__((packed)) M_Hello {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;
  uint16_t sender_id : 14;
  uint8_t rez : 2;              
  uint32_t loc_timer : 32;                                          
  char sender_name[16]; // Фиксированный размер для структуры (size_name + 1)
};

// Онлайн статус (5 байт)
struct __attribute__((packed)) M_Online {
  TypeMessage typeM : 4;
  FlagMessage flagM : 4;
  uint8_t hop : 4;
  uint16_t id_message : 12;                 
  uint16_t sender_id : 14;
  uint8_t rez : 2;              
};

struct __attribute__((packed)) M_Data {
  TypeData type : 4;          
  uint32_t size : 28;         
  uint8_t data[];            
};

struct Message { M_Head Head; M_Data Data; };
struct PartMessage { M_Head_Part Head; M_Data Data; };

// --- АРХИВ ---
// Не используется с версии 0.0.17
struct __attribute__((packed)) ReactionDef { uint8_t id; char emoji; char hint; };
// Не используется с версии 5.7.0
struct __attribute__((packed)) M_Online_Legacy { uint16_t sender_id : 16; };
// Не используется с версии 6.0.1 (Перенесено в UserList.h)
// struct NetSettings { ... }; 

#endif

/* СПИСОК НЕИСПОЛЬЗУЕМЫХ ЭЛЕМЕНТОВ protocol.h:
 * - struct ReactionDef   | с версии 0.0.17
 * - struct M_Online_Legacy| с версии 5.7.0
 * - struct NetSettings   | с версии 6.0.1
 */
