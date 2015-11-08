#include <pebble.h>
#include "ticks.h"
#include "common.h"
#include "geo.h"

#define TICK_COLOR GColorLightGray
#define TICK_STROKE 2
#define TICK_LENGTH 6

static Layer * s_tick_layer;
static GRect s_bounds;
static GPoint s_center;

static Vector tick_vector(float angle){
  const int radius_ext = radius_to_border(angle, &s_bounds.size);
  const GPoint ext = gpoint_on_circle(s_center, angle, radius_ext);
  const int radius_origin = radius_ext - TICK_LENGTH;
  const GPoint ori = gpoint_on_circle(s_center, angle, radius_origin);
  return (Vector){
    .ori = ori,
    .ext = ext
  };
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  const Time current_time = get_current_time();
  const float hour_tick_angle = angle(current_time.hour, 12);
  Vector vector = tick_vector(hour_tick_angle);
  set_stroke_color(ctx, TICK_COLOR);
  set_stroke_width(ctx, TICK_STROKE);
  draw_line(ctx, vector.ori, vector.ext);
  if(current_time.minute / 5 == current_time.hour){
    return;
  }
  const float minute_tick_angle = angle(current_time.minute / 5, 12);
  vector = tick_vector(minute_tick_angle);
  draw_line(ctx, vector.ori, vector.ext);
}

void init_tick_layer(Layer * root_layer){
  s_bounds = layer_get_bounds(root_layer);
  s_center = grect_center_point(&s_bounds);
  
  s_tick_layer = layer_create(s_bounds);
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
