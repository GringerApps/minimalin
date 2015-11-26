#include <pebble.h>
#include "geo.h"
#include "times.h"
#include "ticks.h"
#include "hands.h"
#include "bluetooth.h"
#include "background_layer.h"
#include "common.h"
#include "config.h"

static Window * s_main_window;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){ 
  update_current_time();
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_time_changed();
}

static void config_updated_callback(){
  window_set_background_color(s_main_window, config_get_background_color());
  Layer * root_layer = window_get_root_layer(s_main_window);
  layer_mark_dirty(root_layer);
  hands_update_rainbow_mode_config_changed();
}

static void main_window_load(Window *window) {
  update_current_time();
  init_config(config_updated_callback);
  Layer * root_layer = window_get_root_layer(window);
  init_font();
  init_background_layer(root_layer);
  init_bluetooth_layer(root_layer);
  init_times(root_layer);
  init_tick_layer(root_layer);
  init_hands(root_layer);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
  deinit_hands();
  deinit_times();
  deinit_tick_layer();
  deinit_bluetooth_layer();
  deinit_background_layer();
  deinit_font();
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
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
