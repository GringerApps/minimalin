#include <pebble.h>
#include "config.h"
#include "macros.h"
#include "hand_layer.h"

#define up_to(i, n) for(int i = 0; i < n; ++i)

static Config s_config;
static int s_minute_hand_color_keys[] = { KEY_MINUTE_HAND_COLOR_RED, KEY_MINUTE_HAND_COLOR_GREEN, KEY_MINUTE_HAND_COLOR_BLUE };

// Utils

static void log_color_change(const char config_name[],const int new_values[]){
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "%s color changed changed to 0x%x%x%x",
    config_name,
    new_values[0],
    new_values[1],
    new_values[2]
  );
}

static void write_color(const int keys[3], const int color[3]){
  up_to(i, 3){
    persist_write_int(keys[i], color[i]);
  }
}

static void read_color(const int keys[3], int color[3]){
  up_to(i, 3){
    color[i] = persist_read_int(keys[i]);
  }
}

static void set_minute_hand_color(const int color[3]){
  write_color(s_minute_hand_color_keys, color);
  s_config.minute_hand_color = GColorFromRGB(color[0], color[1], color[2]);
}

// Defaults loading

static void fetch_config_or_default_color(const int keys[3], int default_color[3]){
  up_to(i, 3){
    if(!persist_exists(keys[i])){
      return;
    }
  }
  read_color(keys, default_color);  
}

static void fetch_minute_hand_config_or_default(){
  int colors[3] = {0xff,0xff,0xff};
  fetch_config_or_default_color(s_minute_hand_color_keys, colors);
  set_minute_hand_color(colors);
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
  read_color(keys, existing_values);
  up_to(i, 3){
    if(new_color[i] != existing_values[i]){
      return true;
    }
  }
  return false;
}

static void save_minute_hand_config(const DictionaryIterator *iter){
  int color[3]; 
  if(parse_color_config(iter, s_minute_hand_color_keys, color)){
    log_color_change("Minute hand", color);
    set_minute_hand_color(color);
    mark_dirty_hand_layer();
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  save_minute_hand_config(iter);
}

// API

GColor config_get_minute_hand_color(){
  return s_config.minute_hand_color;
}
    
void init_config() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  fetch_minute_hand_config_or_default();
}
