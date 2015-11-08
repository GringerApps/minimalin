#include <pebble.h>
#include "ticks.h"
#include "time_utils.h"
#include "geo.h"
#include "macros.h"

static Layer * s_tick_layer;

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  Time current_time;
  set_current_time(&current_time);
  GRect screen_bounds = layer_get_bounds(layer);
  float hour_tick_angle = angle(current_time.hours, 12);
  Vector vector = tick_vector(hour_tick_angle, &screen_bounds);
  SSC(ctx, TICK_COLOR);
  SSW(ctx, TICK_STROKE);
  DL(ctx, vector.ori, vector.ext);
  float minute_tick_angle = angle(current_time.minutes / 5, 12);
  if(minute_tick_angle == hour_tick_angle){
    return;
  }
  vector = tick_vector(minute_tick_angle, &screen_bounds);
  SSC(ctx, TICK_COLOR);
  SSW(ctx, TICK_STROKE);
  DL(ctx, vector.ori, vector.ext);
}

void init_tick_layer(Layer * root_layer){
  GRect screen_bounds = layer_get_bounds(root_layer);
  s_tick_layer = layer_create(screen_bounds);
  layer_set_update_proc(s_tick_layer, tick_layer_update_callback);
  layer_add_child(root_layer, s_tick_layer);
}

void deinit_tick_layer(){
  layer_destroy(s_tick_layer); 
}

void mark_dirty_tick_layer(){
  if(s_tick_layer){
    layer_mark_dirty(s_tick_layer);
  }
}