#include <pebble.h>
#include "hands.h"
#include "time_utils.h"
#include "geo.h"
#include "macros.h"
#include "config.h"


static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static Layer * s_center_circle_layer;

static void minute_hand_layer_update_callback(Layer *layer, GContext * ctx){
  GRect screen_bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&screen_bounds);
  Time current_time = get_current_time();
  float minute_angle = angle(current_time.minute, 60);
  if(config_is_rainbow_mode()){
    graphics_draw_rotated_bitmap(ctx, s_rainbow_bitmap, GPoint(5, 55), minute_angle, center);
  }else{
    GPoint minute_hand_end = center;
    translate(minute_angle, MINUTE_HAND_RADIUS, &minute_hand_end);
    SSW(ctx, MINUTE_HAND_STROKE);
    SSC(ctx, config_get_minute_hand_color());
    DL(ctx, center, minute_hand_end);
  }
}

static void hour_hand_layer_update_callback(Layer * layer, GContext * ctx){
  GRect screen_bounds = layer_get_bounds(layer);
  GPoint screen_center = grect_center_point(&screen_bounds);
  Time current_time = get_current_time();
  float hour_angle = angle(current_time.hour, 12);
  float minute_angle = angle(current_time.minute, 60);
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  GPoint hour_hand_end = screen_center;
  translate(hour_angle, HOUR_HAND_RADIUS, &hour_hand_end);
  SSW(ctx, HOUR_HAND_STROKE);
  SSC(ctx, config_get_hour_hand_color());
  DL(ctx, screen_center, hour_hand_end);
}

static void center_circle_layer_update_callback(Layer * layer, GContext * ctx){
  GRect screen_bounds = layer_get_bounds(s_center_circle_layer);
  GPoint center = grect_center_point(&screen_bounds);
  if(config_is_rainbow_mode()){
    graphics_context_set_fill_color(ctx, GColorVividViolet);
  }else{
    graphics_context_set_fill_color(ctx, config_get_hour_hand_color());
  }
  graphics_fill_circle(ctx, center, HOUR_CIRCLE_RADIUS);
}

void init_hand_layer(Layer * root_layer){
  GRect screen_bounds = layer_get_bounds(root_layer);
  s_minute_hand_layer = layer_create(screen_bounds);
  s_hour_hand_layer = layer_create(screen_bounds);
  s_center_circle_layer = layer_create(screen_bounds);
  s_rainbow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_RAINBOW_HAND);
  layer_set_update_proc(s_hour_hand_layer, hour_hand_layer_update_callback);
  layer_set_update_proc(s_minute_hand_layer, minute_hand_layer_update_callback);
  layer_set_update_proc(s_center_circle_layer, center_circle_layer_update_callback);
  layer_add_child(root_layer, s_minute_hand_layer);
  layer_add_child(root_layer, s_hour_hand_layer);
  layer_add_child(root_layer, s_center_circle_layer);
}

void deinit_hand_layer(){
  layer_destroy(s_hour_hand_layer);
  layer_destroy(s_minute_hand_layer);
  layer_destroy(s_center_circle_layer);
  gbitmap_destroy(s_rainbow_bitmap);
}

void mark_dirty_hour_hand_layer(){
  if(s_hour_hand_layer){
    layer_mark_dirty(s_hour_hand_layer);
  }
}

void mark_dirty_minute_hand_layer(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
    layer_mark_dirty(s_center_circle_layer);
  }
}
