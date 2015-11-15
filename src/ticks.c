#include <pebble.h>
#include "ticks.h"
#include "common.h"
#include "geo.h"

#define TICK_COLOR GColorLightGray
#define TICK_STROKE 2
#define TICK_LENGTH 6

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

static Layer * s_tick_layer;
static GRect s_bounds;
static GPoint s_center;

static void draw_tick(GContext *ctx, const int index){
  draw_line(ctx, ticks_points[index][0], ticks_points[index][1]);
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  const Time current_time = get_current_time();
  set_stroke_color(ctx, TICK_COLOR);
  set_stroke_width(ctx, TICK_STROKE);
  const int hour_tick_index = current_time.hour;
  draw_tick(ctx, hour_tick_index);
  const int minute_tick_index = current_time.minute / 5;
  if(hour_tick_index != minute_tick_index){
    draw_tick(ctx, minute_tick_index);
  }

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
