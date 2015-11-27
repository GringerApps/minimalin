#include <pebble.h>
#include "bluetooth.h"
#include "config.h"
#include "common.h"

static const int ICON_OFFSET = -18;

static GBitmap * s_icn_bt;
static GBitmap * s_icn_heart;
static BitmapLayer * s_bt_layer;
static BitmapLayer * s_heart_layer;
static Layer * s_root_layer;

static void hide_layer(const BitmapLayer * layer, const bool hidden){
  layer_set_hidden(bitmap_layer_get_layer(layer), hidden);
}

static void bt_handler(bool connected){
  const BluetoothIcon new_icon = config_get_bluetooth_icon();
  if(connected || new_icon == NoIcon){
    hide_layer(s_bt_layer, true);
    hide_layer(s_heart_layer, true);
  }else{
    hide_layer(s_bt_layer, new_icon != Bluetooth);
    hide_layer(s_heart_layer, new_icon != Heart);
  }
}

static GRect icon_bounds(const GPoint center, const GBitmap * bitmap){
  const GSize size = gbitmap_get_bounds(bitmap).size;
  const GRect rect_at_center = (GRect) { .origin = center, .size = size };
  return grect_translated(rect_at_center, - size.w / 2, - size.h + ICON_OFFSET);
}

static void setup_bitmap(GBitmap ** bitmap, BitmapLayer ** layer, const int key){
  const GRect root_layer_bounds = layer_get_bounds(s_root_layer);
  const GPoint center = grect_center_point(&root_layer_bounds);
  *bitmap = gbitmap_create_with_resource(key);
  const GRect bounds = icon_bounds(center, *bitmap);
  *layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(*layer, *bitmap);
  bitmap_layer_set_compositing_mode(*layer, GCompOpSet);
  layer_add_child(s_root_layer, bitmap_layer_get_layer(*layer));
}

void init_bluetooth_layer(Layer * root_layer){
  s_root_layer = root_layer;
  setup_bitmap(&s_icn_bt, &s_bt_layer, RESOURCE_ID_ICN_BLUETOOTH);
  setup_bitmap(&s_icn_heart, &s_heart_layer, RESOURCE_ID_ICN_HEART);
  bluetooth_connection_service_subscribe(bt_handler);
}

void deinit_bluetooth_layer(){
  bluetooth_connection_service_unsubscribe();
  bitmap_layer_destroy(s_heart_layer);
  gbitmap_destroy(s_icn_heart);
  bitmap_layer_destroy(s_bt_layer);
  gbitmap_destroy(s_icn_bt);
}

void mark_dirty_bluetooth_layer(){
  bt_handler(connection_service_peek_pebble_app_connection());
}
