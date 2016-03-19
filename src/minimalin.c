#include <pebble.h>
#include "times.h"
#include "ticks.h"
#include "hands.h"
#include "common.h"
#include "config.h"

static Window * s_main_window;

static const int ICON_OFFSET = -18;

static TextLayer * s_bt_layer;
static Layer * s_root_layer;



// Info layer: bluetooth

static void mark_dirty_info_layer();

static void bt_handler(bool connected){
  const GColor bg_color = config_get_background_color();
  bool bg_reddish = false;
  const GColor reddish_colors[] = { GColorRed, GColorFolly, GColorFashionMagenta, GColorMagenta };
  for(int i = 0; i < 4; i++){
    if(gcolor_equal(bg_color, reddish_colors[i])){
      bg_reddish = true;
      break;
    }
  }
  if(bg_reddish){
    text_layer_set_text_color(s_bt_layer, GColorWhite);
  }else{
    text_layer_set_text_color(s_bt_layer, GColorRed);
  }
  const BluetoothIcon new_icon = config_get_bluetooth_icon();
  if(connected || new_icon == NoIcon){
    text_layer_set_text(s_bt_layer, "");
  }else if(new_icon == Bluetooth){
    text_layer_set_text(s_bt_layer, "μ");
  }else{
    text_layer_set_text(s_bt_layer, "ν");
  }
}

static void mark_dirty_info_layer(){
  bt_handler(connection_service_peek_pebble_app_connection());
}

static void init_info_layer(Layer * root_layer){
  s_root_layer = root_layer;
  const GRect root_layer_bounds = layer_get_bounds(root_layer);
  const GPoint center = grect_center_point(&root_layer_bounds);
  const GSize size = GSize(23, 23);
  const GRect rect_at_center = (GRect) { .origin = center, .size = size };
  const GRect bounds = grect_translated(rect_at_center, - size.w / 2, - size.h + ICON_OFFSET);
  s_bt_layer = text_layer_create(bounds);
  text_layer_set_text_alignment(s_bt_layer, GTextAlignmentCenter);
  text_layer_set_font(s_bt_layer, get_font());
  text_layer_set_overflow_mode(s_bt_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_bt_layer, GColorClear);
  layer_add_child(root_layer, text_layer_get_layer(s_bt_layer));
  bluetooth_connection_service_subscribe(bt_handler);
  mark_dirty_info_layer();
}

static void deinit_info_layer(){
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_bt_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_current_time();
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_time_changed();
}

static void config_updated_callback(){
  Layer * root_layer = window_get_root_layer(s_main_window);
  layer_mark_dirty(root_layer);
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_rainbow_mode_config_changed();
  window_set_background_color(s_main_window, config_get_background_color());
}

static void main_window_load(Window *window) {
  update_current_time();
  window_set_background_color(window, config_get_background_color());
  Layer * root_layer = window_get_root_layer(window);
  init_font();
  init_info_layer(root_layer);
  init_times(root_layer);
  init_tick_layer(root_layer);
  init_hands(root_layer);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
  deinit_hands();
  deinit_times();
  deinit_tick_layer();
  deinit_info_layer();
  deinit_font();
}

static void init() {
  init_config(config_updated_callback);
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
