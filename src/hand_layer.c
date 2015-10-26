#include <pebble.h>
#include "hand_layer.h"
#include "time_utils.h"
#include "geo.h"
#include "macros.h"
#include "config.h"

static Layer * s_hand_layer;

static void hand_layer_update_callback(Layer *layer, GContext *ctx) {
  GRect screen_bounds = layer_get_bounds(layer);
  GPoint screen_center = grect_center_point(&screen_bounds);
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  Time current_time;
  set_current_time(&current_time);
  float minute_angle = angle(current_time.minutes, 60);
  float hour_angle = angle(current_time.hours, 12);
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  GPoint minute_hand_end = screen_center;
  translate(minute_angle, MINUTE_HAND_RADIUS, &minute_hand_end);
  GPoint hour_hand_end = screen_center;
  translate(hour_angle, HOUR_HAND_RADIUS, &hour_hand_end);
  SAA(ctx, true);
  SSW(ctx, MINUTE_HAND_STROKE);
  SSC(ctx, config_get_minute_hand_color());
  DL(ctx, screen_center, minute_hand_end);
  SSC(ctx, config_get_hour_hand_color());
  DL(ctx, screen_center, hour_hand_end);
  SFC(ctx, config_get_hour_hand_color());
  FC(ctx, screen_center, HOUR_CIRCLE_RADIUS);
}

void init_hand_layer(Layer * root_layer){
  GRect screen_bounds = layer_get_bounds(root_layer);
  s_hand_layer = layer_create(screen_bounds);
  layer_set_update_proc(s_hand_layer, hand_layer_update_callback);
  layer_add_child(root_layer, s_hand_layer);
}

void deinit_hand_layer(){
  layer_destroy(s_hand_layer); 
}

void mark_dirty_hand_layer(){
  if(s_hand_layer){
    layer_mark_dirty(s_hand_layer);
  }
}