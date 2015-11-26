#include <pebble.h>
#include "config.h"

#define MESSAGE_KEY_MINUTE_HAND_COLOR 0
#define MESSAGE_KEY_HOUR_HAND_COLOR   1
#define MESSAGE_KEY_DATE_DISPLAYED    2
#define MESSAGE_KEY_BLUETOOTH_ICON    3
#define MESSAGE_KEY_RAINBOW_MODE      4
#define MESSAGE_KEY_BACKGROUND_COLOR  5
#define MESSAGE_KEY_DATE_COLOR        6
#define MESSAGE_KEY_TIME_COLOR        7
#define PERSIST_KEY_CONFIG            0

typedef struct {
  int32_t minute_hand_color;
  int32_t hour_hand_color;
  int32_t background_color;
  int32_t date_color;
  int32_t time_color;
  int8_t date_displayed;
  int8_t bluetooth_icon;
  int8_t rainbow_mode;
} __attribute__((__packed__)) Config;

static Config s_config;
static ConfigUpdatedCallback s_config_updated_callback;

static void fetch_int32(const DictionaryIterator * iter, const int key, int32_t * config){
  Tuple * tuple = dict_find(iter, key);
  if(tuple){
    *config = tuple->value->int32;
  }
}

static void fetch_int8(const DictionaryIterator * iter, const int key, int8_t * config){
  Tuple * tuple = dict_find(iter, key);
  if(tuple){
    *config = tuple->value->int8;
  }
}

static void fetch_config_or_default(){
  if(persist_exists(PERSIST_KEY_CONFIG)){
    persist_read_data(PERSIST_KEY_CONFIG, &s_config, sizeof(Config));
  }else{
    s_config.minute_hand_color = 0xffffff;
    s_config.hour_hand_color   = 0xff0000;
    s_config.background_color  = 0x000000;
    s_config.date_color        = 0x555555;
    s_config.time_color        = 0xAAAAAA;
    s_config.date_displayed    = true;
    s_config.bluetooth_icon    = Bluetooth;
    s_config.rainbow_mode      = false;
    persist_write_data(PERSIST_KEY_CONFIG, &s_config, sizeof(s_config));
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  fetch_int32(iter, MESSAGE_KEY_MINUTE_HAND_COLOR, &s_config.minute_hand_color);
  fetch_int32(iter, MESSAGE_KEY_HOUR_HAND_COLOR, &s_config.hour_hand_color);
  fetch_int32(iter, MESSAGE_KEY_BACKGROUND_COLOR, &s_config.background_color);
  fetch_int32(iter, MESSAGE_KEY_DATE_COLOR, &s_config.date_color);
  fetch_int32(iter, MESSAGE_KEY_TIME_COLOR, &s_config.time_color);
  fetch_int8(iter, MESSAGE_KEY_DATE_DISPLAYED, &s_config.date_displayed);
  fetch_int8(iter, MESSAGE_KEY_BLUETOOTH_ICON, &s_config.bluetooth_icon);
  fetch_int8(iter, MESSAGE_KEY_RAINBOW_MODE, &s_config.rainbow_mode);
  persist_write_data(PERSIST_KEY_CONFIG, &s_config, sizeof(s_config));
  s_config_updated_callback();
}

GColor config_get_minute_hand_color(){
  return GColorFromHEX(s_config.minute_hand_color);
}

GColor config_get_hour_hand_color(){
  return GColorFromHEX(s_config.hour_hand_color);
}

GColor config_get_background_color(){
  return GColorFromHEX(s_config.background_color);
}

GColor config_get_date_color(){
  return GColorFromHEX(s_config.date_color);
}

GColor config_get_time_color(){
  return GColorFromHEX(s_config.time_color);
}

bool config_is_date_displayed(){
  return s_config.date_displayed;
}

BluetoothIcon config_get_bluetooth_icon(){
  return s_config.bluetooth_icon;
}

bool config_is_rainbow_mode(){
  return s_config.rainbow_mode;
}

void init_config(ConfigUpdatedCallback callback){
  s_config_updated_callback = callback;
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  fetch_config_or_default();
}
