// Core: ESP8266 v2.7.4 | v.1.8.5 | ПОЛНЫЙ БИНАРНЫЙ ПРОТОКОЛ ТАБУ
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <Arduino.h>

const uint8_t size_message_id = 12; 
const uint8_t size_user_id    = 16;
const uint8_t size_name       = 20;

#define MAX_REACTIONS_DB 16
#define CFG_FILENAME_LEN 16

enum TypeMessage : uint8_t { 
  MSG_DATA=1, MSG_COMPOUND=2, MSG_ONLINE=3, MSG_CNG_STATUS=4, 
  MSG_CONFIRMATION=5, MSG_HELLO=6, MSG_POLL=8, MSG_CONFIG=9 
};

enum FlagMessage : uint8_t { 
  MSG_SEND=1, MSG_RECEIVED=2, MSG_DELIVERED=3, MSG_REDELIVERED=4, 
  MSG_READ=5, MSG_USER_SUSPEND=6, MSG_USER_EXIT=7, MSG_TASK=8 
};

enum TypeData : uint8_t { DT_TEXT=1, DT_AUDIO=2, DT_IMAGE=3 };

struct __attribute__((packed)) NetSettings {
  char ssid[32];
  char pass[64];
  uint32_t ip;
  uint32_t gw;
  uint32_t mask;
  uint8_t isAP; 
};

struct __attribute__((packed)) ReactionDef {
  uint8_t id; char emoji[8]; char hint[32];
};

struct __attribute__((packed)) M_Head { 
  uint8_t typeM:4; uint8_t flagM:4; uint8_t hop:4; uint16_t id_message:12; 
  uint16_t destination_id; uint16_t sender_id; 
};

struct __attribute__((packed)) M_Head_Part { 
  uint8_t typeM:4; uint8_t flagM:4; uint8_t hop:4; uint16_t id_message:12; 
  uint16_t destination_id; uint16_t sender_id; 
  uint8_t count_frame:4; uint8_t num_frame:4; uint8_t rez:4; uint16_t id_orign_message:12; 
};

struct __attribute__((packed)) M_Online {
    uint8_t typeM : 4; uint8_t flagM : 4; uint8_t hop : 4;
    uint16_t id_message : 12; uint16_t sender_id; uint8_t rez : 4;
}; 

struct __attribute__((packed)) M_Hello {
  uint8_t typeM : 4; uint8_t flagM : 4; uint8_t hop : 4;
  uint16_t id_message : 12; uint8_t rez : 4; uint16_t sender_id;
  uint32_t loc_timer; char sender_name[size_name + 1];
};

struct __attribute__((packed)) M_Poll { 
  uint8_t typeM:4; uint8_t flagM:4; uint8_t hop:4; uint16_t id_message:12; 
  uint16_t sender_id; uint16_t votes_mask; 
};

struct __attribute__((packed)) M_Config {
  uint8_t typeM:4; uint8_t flagM:4; uint8_t hop:4; uint16_t id_message:12;
  uint8_t reboot:1; uint8_t rez:3; uint8_t rez2:4;
  char filename[CFG_FILENAME_LEN]; uint32_t reboot_delay; uint32_t data_size;    
};

struct __attribute__((packed)) M_Data { uint32_t type:4; uint32_t size:28; uint8_t data[]; };
#endif

