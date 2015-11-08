#include <pebble.h>
#include "config.h"
#include "hands.h"
#include "times.h"
#include "bluetooth.h"

#define KEY_MINUTE_HAND_COLOR_RED   0
#define KEY_MINUTE_HAND_COLOR_GREEN 1
#define KEY_MINUTE_HAND_COLOR_BLUE  2
#define KEY_HOUR_HAND_COLOR_RED     3
#define KEY_HOUR_HAND_COLOR_GREEN   4
#define KEY_HOUR_HAND_COLOR_BLUE    5
#define KEY_DATE_DISPLAYED          6
#define KEY_BLUETOOTH_DISPLAYED     7
#define KEY_RAINBOW_MODE            8
#define up_to(i, n) for(int i = 0; i < n; ++i)

static Config s_config;
static const int s_minute_hand_color_keys[] = { KEY_MINUTE_HAND_COLOR_RED, KEY_MINUTE_HAND_COLOR_GREEN, KEY_MINUTE_HAND_COLOR_BLUE };
static const int s_hour_hand_color_keys[] = { KEY_HOUR_HAND_COLOR_RED, KEY_HOUR_HAND_COLOR_GREEN, KEY_HOUR_HAND_COLOR_BLUE };

// Utils

static void write_color(const int keys[3], const int color[3]){
  up_to(i, 3){
    persist_write_int(keys[i], color[i]);
  }
}

static bool read_color(const int keys[3], int color[3]){
  up_to(i, 3){
    if(!persist_exists(keys[i])){
      return false;
    }
  }
  up_to(i, 3){
    color[i] = persist_read_int(keys[i]);
  }
  return true;
}

static void set_color(const int keys[3], const int color[3]){
  write_color(keys, color);
  GColor g_color = GColorFromRGB(color[0], color[1], color[2]);
  switch(keys[0]){
  case KEY_MINUTE_HAND_COLOR_RED:
    s_config.minute_hand_color = g_color;
    return;
  default:
    s_config.hour_hand_color = g_color;
  }
}

static void set_bool(const int key, const bool value){
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

// Defaults loading

static void fetch_color_config_or_default(const int keys[3], const int default_color[3]){
  int color[3];
  memcpy(color, default_color, 3 * sizeof(int));
  read_color(keys, color);
  set_color(keys, color);
}

static void fetch_bool_config_or_default(const int key, const int default_value){
  int value = default_value;
  if(persist_exists(key)){
    value = persist_read_bool(key);
  }
  set_bool(key, value);
}

static void fetch_config_or_default(){
  const int default_minute_hand_color[3] = {0xff,0xff,0xff};
  fetch_color_config_or_default(s_minute_hand_color_keys, default_minute_hand_color);
  const int default_hour_hand_color[3] = {0xff,0x00, 0x00};
  fetch_color_config_or_default(s_hour_hand_color_keys, default_hour_hand_color);
  fetch_bool_config_or_default(KEY_DATE_DISPLAYED, true);
  fetch_bool_config_or_default(KEY_BLUETOOTH_DISPLAYED, true);
  fetch_bool_config_or_default(KEY_RAINBOW_MODE, false);
}

// Config change

static bool parse_color_config(const DictionaryIterator *iter, const int keys[3], int new_color[3]){
  up_to(i, 3){
    Tuple * new_value = dict_find(iter, keys[i]);
    if(!new_value){
      return false;
    }
    new_color[i] = new_value->value->int32;
  }
  int existing_values[3];
  if(read_color(keys, existing_values)){
    up_to(i, 3){
      if(new_color[i] != existing_values[i]){
        return true;
      }
    }
    return false;
  }
  return true;
}

static void save_minute_hand_config(const DictionaryIterator *iter){
  int color[3]; 
  if(parse_color_config(iter, s_minute_hand_color_keys, color)){
    set_color(s_minute_hand_color_keys, color);
    hands_update_minute_hand_config_changed();
  }
}

static void save_hour_hand_config(const DictionaryIterator *iter){
  int color[3]; 
  if(parse_color_config(iter, s_hour_hand_color_keys, color)){
    set_color(s_hour_hand_color_keys, color);
    hands_update_hour_hand_config_changed();
  }
}

static void save_date_displayed_config(const DictionaryIterator *iter){
  Tuple * new_value = dict_find(iter, KEY_DATE_DISPLAYED);
  if(new_value){
    set_bool(KEY_DATE_DISPLAYED, new_value->value->int8);
    mark_dirty_time_layer();
  }
}

static void save_bluetooth_displayed_config(const DictionaryIterator *iter){
  Tuple * new_value = dict_find(iter, KEY_BLUETOOTH_DISPLAYED);
  if(new_value){
    set_bool(KEY_BLUETOOTH_DISPLAYED, new_value->value->int8);
    mark_dirty_bluetooth_layer();
  }
}

static void save_rainbow_mode_config(const DictionaryIterator *iter){
  Tuple * new_value = dict_find(iter, KEY_RAINBOW_MODE);
  if(new_value){
    set_bool(KEY_RAINBOW_MODE, new_value->value->int8);
    hands_update_rainbow_mode_config_changed();
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  save_minute_hand_config(iter);
  save_hour_hand_config(iter);
  save_date_displayed_config(iter);
  save_bluetooth_displayed_config(iter);
  save_rainbow_mode_config(iter);
}

// API

GColor config_get_minute_hand_color(){
  return s_config.minute_hand_color;
}

GColor config_get_hour_hand_color(){
  return s_config.hour_hand_color;
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
