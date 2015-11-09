#include <pebble.h>
#include "geo.h"
#include "macros.h"  
#include "time_layer.h"
#include "tick_layer.h"
#include "hand_layer.h"
#include "bluetooth_layer.h"
#include "background_layer.h"
#include "time_utils.h"
#include "config.h"

static Window *s_main_window;

static void main_window_load(Window *window) {
  Layer * root_layer = window_get_root_layer(window);  
  init_background_layer(root_layer);
  init_bluetooth_layer(root_layer);
  init_time_layer(root_layer);
  init_tick_layer(root_layer);
  init_hand_layer(root_layer);
}

static void main_window_unload(Window *window) {
  deinit_hand_layer();
  deinit_time_layer();
  deinit_tick_layer();
  deinit_bluetooth_layer();
  deinit_background_layer();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){ 
  update_current_time();
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  mark_dirty_hour_hand_layer();
  mark_dirty_minute_hand_layer();
}

static void init() {
  init_config();
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
