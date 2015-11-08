#include <pebble.h>
#include "hands.h"
#include "time_utils.h"
#include "geo.h"
#include "macros.h"
#include "config.h"

static GPoint s_center;
static GRect s_bounds;
static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static Layer * s_center_circle_layer;

static void update_minute_hand_layer(Layer *layer, GContext * ctx){
  Time current_time = get_current_time();
  float minute_angle = angle(current_time.minute, 60);
  if(config_is_rainbow_mode()){
    graphics_draw_rotated_bitmap(ctx, s_rainbow_bitmap, GPoint(5, 55), minute_angle, s_center);
  }else{
    GPoint minute_hand_end = s_center;
    translate(minute_angle, MINUTE_HAND_RADIUS, &minute_hand_end);
    SSW(ctx, MINUTE_HAND_STROKE);
    SSC(ctx, config_get_minute_hand_color());
    DL(ctx, s_center, minute_hand_end);
  }
}

static void update_hour_hand_layer(Layer * layer, GContext * ctx){
  Time current_time = get_current_time();
  float hour_angle = angle(current_time.hour, 12);
  float minute_angle = angle(current_time.minute, 60);
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  GPoint hour_hand_end = s_center;
  translate(hour_angle, HOUR_HAND_RADIUS, &hour_hand_end);
  SSW(ctx, HOUR_HAND_STROKE);
  SSC(ctx, config_get_hour_hand_color());
  DL(ctx, s_center, hour_hand_end);
}

static void update_center_circle_layer(Layer * layer, GContext * ctx){
  if(config_is_rainbow_mode()){
    graphics_context_set_fill_color(ctx, GColorVividViolet);
  }else{
    graphics_context_set_fill_color(ctx, config_get_hour_hand_color());
  }
  graphics_fill_circle(ctx, s_center, HOUR_CIRCLE_RADIUS);
}

void init_hands(Layer * root_layer){
  s_bounds = layer_get_bounds(root_layer);
  s_center = grect_center_point(&s_bounds);
  
  s_minute_hand_layer   = layer_create(s_bounds);
  s_hour_hand_layer     = layer_create(s_bounds);
  s_center_circle_layer = layer_create(s_bounds);
  
  layer_set_update_proc(s_hour_hand_layer,     update_hour_hand_layer);
  layer_set_update_proc(s_minute_hand_layer,   update_minute_hand_layer);
  layer_set_update_proc(s_center_circle_layer, update_center_circle_layer);
  
  layer_add_child(root_layer, s_minute_hand_layer);
  layer_add_child(root_layer, s_hour_hand_layer);
  layer_add_child(root_layer, s_center_circle_layer);

  s_rainbow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_RAINBOW_HAND);
}

void deinit_hands(){
  layer_destroy(s_hour_hand_layer);
  layer_destroy(s_minute_hand_layer);
  layer_destroy(s_center_circle_layer);
  gbitmap_destroy(s_rainbow_bitmap);
}

void hands_update_time_changed(){
 if(s_hour_hand_layer){
   layer_mark_dirty(s_hour_hand_layer);
 }
 if(s_minute_hand_layer){
   layer_mark_dirty(s_minute_hand_layer);
 }
}

void hands_update_minute_hand_config_changed(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
  }
}

void hands_update_hour_hand_config_changed(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
  }
}

void hands_update_rainbow_mode_config_changed(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
  }
  if(s_center_circle_layer){
    layer_mark_dirty(s_center_circle_layer);
  }
}
