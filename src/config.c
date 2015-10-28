#include <pebble.h>
#include "config.h"
#include "macros.h"
#include "hand_layer.h"
#include "time_layer.h"

#define up_to(i, n) for(int i = 0; i < n; ++i)

static Config s_config;
static int s_minute_hand_color_keys[] = { KEY_MINUTE_HAND_COLOR_RED, KEY_MINUTE_HAND_COLOR_GREEN, KEY_MINUTE_HAND_COLOR_BLUE };
static int s_hour_hand_color_keys[] = { KEY_HOUR_HAND_COLOR_RED, KEY_HOUR_HAND_COLOR_GREEN, KEY_HOUR_HAND_COLOR_BLUE };

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

static void set_hour_hand_color(const int color[3]){
  write_color(s_hour_hand_color_keys, color);
  s_config.hour_hand_color = GColorFromRGB(color[0], color[1], color[2]);
}

static void set_date_displayed(const bool displayed){
  persist_write_bool(KEY_DATE_DISPLAYED, displayed);
  s_config.date_displayed = displayed;
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

static void fetch_hour_hand_config_or_default(){
  int colors[3] = {0xff,0x00,0x00};
  fetch_config_or_default_color(s_hour_hand_color_keys, colors);
  set_hour_hand_color(colors);
}

static void fetch_date_display_config_or_default(){
  bool is_date_displayed = true;
  if(persist_exists(KEY_DATE_DISPLAYED)){
    is_date_displayed = persist_read_bool(KEY_DATE_DISPLAYED);
  }
  set_date_displayed(is_date_displayed);
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

static void save_hour_hand_config(const DictionaryIterator *iter){
  int color[3]; 
  if(parse_color_config(iter, s_hour_hand_color_keys, color)){
    log_color_change("Hour hand", color);
    set_hour_hand_color(color);
    mark_dirty_hand_layer();
  }
}

static void save_date_displayed_config(const DictionaryIterator *iter){
  Tuple * new_value = dict_find(iter, KEY_DATE_DISPLAYED);
  if(new_value){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%d",(int) new_value->value->int8);
    set_date_displayed(new_value->value->int8);
    mark_dirty_time_layer();
  }
}
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  save_minute_hand_config(iter);
  save_hour_hand_config(iter);
  save_date_displayed_config(iter);
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
    
void init_config() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  fetch_minute_hand_config_or_default();
  fetch_hour_hand_config_or_default();
  fetch_date_display_config_or_default();
}
