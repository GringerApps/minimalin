#include <pebble.h>
#include "ticks.h"
#include "common.h"
#include "geo.h"

#define TICK_COLOR GColorLightGray
#define TICK_STROKE 2
#define TICK_LENGTH 6

static Layer * s_tick_layer;

static Vector tick_vector(float angle, const GRect * screen_bounds){
  GPoint center      = grect_center_point(screen_bounds);
  const GSize * size = &screen_bounds->size; 
  GPoint ext;
  int32_t radius_tick_end = radius_to_border(angle, size);
  if(use_cos(angle)){
    ext.x = x_plus_dx(center.x, angle, radius_tick_end),
    ext.y = angle < _45_DEGREES || angle > _315_DEGREES ? 0 : size->h;
  }else{
    ext.y = y_plus_dy(center.y, angle, radius_tick_end),
    ext.x = angle > _135_DEGREES ? 0 : size->w;      
  }
  int32_t radius_tick_start = radius_tick_end - TICK_LENGTH;
  GPoint ori = center;
  translate(angle, radius_tick_start, &ori);
  return (Vector){
    .ori = ori,
    .ext = ext
  };
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  Time current_time = get_current_time();
  GRect screen_bounds = layer_get_bounds(layer);
  float hour_tick_angle = angle(current_time.hour, 12);
  Vector vector = tick_vector(hour_tick_angle, &screen_bounds);
  set_stroke_color(ctx, TICK_COLOR);
  set_stroke_width(ctx, TICK_STROKE);
  draw_line(ctx, vector.ori, vector.ext);
  float minute_tick_angle = angle(current_time.minute / 5, 12);
  if(minute_tick_angle == hour_tick_angle){
    return;
  }
  vector = tick_vector(minute_tick_angle, &screen_bounds);
  set_stroke_color(ctx, TICK_COLOR);
  set_stroke_width(ctx, TICK_STROKE);
  draw_line(ctx, vector.ori, vector.ext);
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
