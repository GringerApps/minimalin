#include <pebble.h>
#include "geo.h"
#include "time_layer.h"
#include "ticks.h"
#include "hands.h"
#include "bluetooth.h"
#include "background_layer.h"
#include "common.h"
#include "config.h"

static Window * s_main_window;

static void main_window_load(Window *window) {
  Layer * root_layer = window_get_root_layer(window);
  init_font();
  init_background_layer(root_layer);
  init_bluetooth_layer(root_layer);
  init_time_layer(root_layer);
  init_tick_layer(root_layer);
  init_hands(root_layer);
}

static void main_window_unload(Window *window) {
  deinit_hands();
  deinit_time_layer();
  deinit_tick_layer();
  deinit_bluetooth_layer();
  deinit_background_layer();
  deinit_font();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){ 
  update_current_time();
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_time_changed();
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
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
