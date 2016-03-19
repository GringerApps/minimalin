#include <pebble.h>
#include "times.h"
#include "hands.h"
#include "common.h"
#include "config.h"

#ifdef PBL_ROUND
static GPoint ticks_points[12][2] = {
  {{90, 0}  , {90, 6}  },
  {{135,12} , {132,18}  },
  {{168,45} , {162,48} },
  {{180,90} , {174,90} },
  {{168,135}, {162,132}},
  {{135,168}, {132,162}},
  {{90, 180}, {90, 174}},
  {{45, 168}, {48, 162}},
  {{12, 135}, {18, 132}},
  {{0,  90} , {6,  90} },
  {{12, 45} , {18, 48} },
  {{45, 12} , {48, 18}  }
};
#else
static GPoint ticks_points[12][2] = {
  {{72, 0}  , {72, 7}  },
  {{120,0}  , {117,7}  },
  {{144,42} , {137,46} },
  {{144,84} , {137,84} },
  {{144,126}, {137,122}},
  {{120,168}, {117,161}},
  {{72, 168}, {72, 161}},
  {{24, 168}, {27, 161}},
  {{0,  126}, {7,  122}},
  {{0,  84} , {7,  84} },
  {{0,  42} , {7,  46} },
  {{24, 0}  , {27, 7}  }
};
#endif
static const int ICON_OFFSET = -18;
static const int TICK_STROKE = 2;
static const int TICK_LENGTH = 6;

static Window * s_main_window;
static TextLayer * s_bt_layer;
static Layer * s_root_layer;
static Layer * s_tick_layer;
static GRect s_bounds;
static GPoint s_center;

// Ticks
static void mark_dirty_tick_layer();

static void draw_tick(GContext *ctx, const int index){
  draw_line(ctx, ticks_points[index][0], ticks_points[index][1]);
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  const Time current_time = get_current_time();
  set_stroke_color(ctx, config_get_time_color());
  set_stroke_width(ctx, TICK_STROKE);
  const int hour_tick_index = current_time.hour % 12;
  draw_tick(ctx, hour_tick_index);
  const int minute_tick_index = current_time.minute / 5;
  if(hour_tick_index != minute_tick_index){
    draw_tick(ctx, minute_tick_index);
  }
}

static void init_tick_layer(Layer * root_layer){
  s_bounds = layer_get_bounds(root_layer);
  s_center = grect_center_point(&s_bounds);

  s_tick_layer = layer_create(s_bounds);
  layer_set_update_proc(s_tick_layer, tick_layer_update_callback);
  layer_add_child(root_layer, s_tick_layer);
}

static void deinit_tick_layer(){
  layer_destroy(s_tick_layer);
}

static void mark_dirty_tick_layer(){
  if(s_tick_layer){
    layer_mark_dirty(s_tick_layer);
  }
}


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
