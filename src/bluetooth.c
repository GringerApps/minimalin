#include <pebble.h>
#include "bluetooth.h"
#include "config.h"

#define ICN_BT_RADIUS 18

typedef enum { NoIcon = 0, Bluetooth = 1, Heart = 2 } BluetoothIcon;

static GBitmap * s_icn_bluetooth;
static GBitmap * s_icn_heart;
static BitmapLayer * s_bluetooth_layer;
static BitmapLayer * s_heart_layer;
static bool s_connected;
static BluetoothIcon s_current_icon = Bluetooth;

static void bt_handler(bool connected){
  const BluetoothIcon new_icon = config_get_bluetooth_icon();
  if(connected || new_icon == NoIcon){
    layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_heart_layer), true);
  }else{
    layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_layer), new_icon != Bluetooth);
    layer_set_hidden(bitmap_layer_get_layer(s_heart_layer), new_icon != Heart);
  }
  s_current_icon = new_icon;
  s_connected = connected;
}

static GRect icon_bounds(const GPoint center, const GBitmap * bitmap){
  const GSize size = gbitmap_get_bounds(bitmap).size;
  GPoint origin = center;
  origin.x -= size.w / 2;
  origin.y -= size.h + ICN_BT_RADIUS;
  return (GRect) { .origin = origin, .size = size };
}

void init_bluetooth_layer(Layer * root_layer){
  s_icn_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_ICN_BLUETOOTH);
  s_icn_heart = gbitmap_create_with_resource(RESOURCE_ID_ICN_HEART);
  
  GRect root_layer_bounds = layer_get_bounds(root_layer);
  GPoint center = grect_center_point(&root_layer_bounds);

  GRect bounds = icon_bounds(center, s_icn_bluetooth);
  s_bluetooth_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_icn_bluetooth);
  bitmap_layer_set_compositing_mode(s_bluetooth_layer, GCompOpSet);
  
  bounds = icon_bounds(center, s_icn_heart);
  s_heart_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_heart_layer, s_icn_heart);
  bitmap_layer_set_compositing_mode(s_heart_layer, GCompOpSet);

  layer_add_child(root_layer, bitmap_layer_get_layer(s_bluetooth_layer));
  layer_add_child(root_layer, bitmap_layer_get_layer(s_heart_layer));

  bt_handler(connection_service_peek_pebble_app_connection());
  bluetooth_connection_service_subscribe(bt_handler);
}

void deinit_bluetooth_layer(){
  bitmap_layer_destroy(s_bluetooth_layer); 
  gbitmap_destroy(s_icn_bluetooth);
  gbitmap_destroy(s_icn_heart);
  bluetooth_connection_service_unsubscribe();
}

void mark_dirty_bluetooth_layer(){
  bt_handler(s_connected);
}

