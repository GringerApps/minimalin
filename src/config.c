#include <pebble.h>
#include "config.h"
#include "hands.h"
#include "times.h"
#include "bluetooth.h"
#include "background_layer.h"

#define KEY_MINUTE_HAND_COLOR   0
#define KEY_HOUR_HAND_COLOR     1
#define KEY_DATE_DISPLAYED      2
#define KEY_BLUETOOTH_DISPLAYED 3
#define KEY_RAINBOW_MODE        4
#define KEY_BACKGROUND_COLOR    5
#define KEY_DATE_COLOR    6
typedef void (*ConfigUpdateCallback)();
typedef void (*ConfigSetter)(const int, const Tuple *);

static Config s_config;

// Utils

static void persist_config_color(const int key, const int color){
  persist_write_int(key, color);
  GColor g_color = GColorFromHEX(color);
  switch(key){
  case KEY_MINUTE_HAND_COLOR:
    s_config.minute_hand_color = g_color;
    return;
  case KEY_HOUR_HAND_COLOR:
    s_config.hour_hand_color = g_color;
    return;
  case KEY_DATE_COLOR:
    s_config.date_color = g_color;
    return;
  default:
    s_config.background_color = g_color;
  }
}

static void persist_config_bool(const int key, const bool value){
  persist_write_bool(key, value);
  switch(key){
  case KEY_DATE_DISPLAYED:
    s_config.date_displayed = value;
    return;
  case KEY_RAINBOW_MODE:
    s_config.rainbow_mode = value;
    return;
  default:
    s_config.bluetooth_displayed = value;
  }
}

static void set_color(const int key, const Tuple * tuple){
  persist_config_color(key, tuple->value->int32);
}

static void set_bool(const int key, const Tuple * tuple){
  persist_config_bool(key, tuple->value->int8);
}

// Defaults loading

static void fetch_color_config_or_default(const int key, const int default_color){
  int color = default_color;
  if(persist_exists(key)){
    color = persist_read_int(key);
  }
  persist_config_color(key, color);
}

static void fetch_bool_config_or_default(const int key, const int default_value){
  int value = default_value;
  if(persist_exists(key)){
    value = persist_read_bool(key);
  }
  persist_config_bool(key, value);
}

static void fetch_config_or_default(){
  fetch_color_config_or_default(KEY_MINUTE_HAND_COLOR, 0xffffff);
  fetch_color_config_or_default(KEY_HOUR_HAND_COLOR, 0xff0000);
  fetch_color_config_or_default(KEY_BACKGROUND_COLOR, 0x000000);
  fetch_color_config_or_default(KEY_DATE_COLOR, 0x555555);
  fetch_bool_config_or_default(KEY_DATE_DISPLAYED, true);
  fetch_bool_config_or_default(KEY_BLUETOOTH_DISPLAYED, true);
  fetch_bool_config_or_default(KEY_RAINBOW_MODE, false);
}

// Config change

static void update_config(const DictionaryIterator *iter, const int key, ConfigSetter setter, ConfigUpdateCallback callback){
  Tuple * new_value = dict_find(iter, key);
  if(new_value){
    setter(key, new_value);
    callback();
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  update_config(iter, KEY_MINUTE_HAND_COLOR, set_color, hands_update_minute_hand_config_changed);
  update_config(iter, KEY_HOUR_HAND_COLOR, set_color, hands_update_hour_hand_config_changed);
  update_config(iter, KEY_BACKGROUND_COLOR, set_color, background_config_changed);
  update_config(iter, KEY_DATE_COLOR, set_color, mark_dirty_time_layer);
  update_config(iter, KEY_DATE_DISPLAYED, set_bool, mark_dirty_time_layer);
  update_config(iter, KEY_BLUETOOTH_DISPLAYED, set_bool, mark_dirty_bluetooth_layer);
  update_config(iter, KEY_RAINBOW_MODE, set_bool, hands_update_rainbow_mode_config_changed);
}

// API

GColor config_get_minute_hand_color(){
  return s_config.minute_hand_color;
}

GColor config_get_hour_hand_color(){
  return s_config.hour_hand_color;
}

GColor config_get_background_color(){
  return s_config.background_color;
}

GColor config_get_date_color(){
  return s_config.date_color;
}

bool config_is_date_displayed(){
  return s_config.date_displayed;
}

bool config_is_bluetooth_displayed(){
  return s_config.bluetooth_displayed;
}

bool config_is_rainbow_mode(){
  return s_config.rainbow_mode;
}
    
void init_config() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  fetch_config_or_default();
}
