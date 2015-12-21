
#include <pebble.h>
#include "bluetooth.h"
#include "config.h"
#include "common.h"

static const int ICON_OFFSET = -18;

static TextLayer * s_bt_layer;
static Layer * s_root_layer;

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

void init_bluetooth_layer(Layer * root_layer){
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
  mark_dirty_bluetooth_layer();
}

void deinit_bluetooth_layer(){
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_bt_layer);
}

void mark_dirty_bluetooth_layer(){
  bt_handler(connection_service_peek_pebble_app_connection());
}
