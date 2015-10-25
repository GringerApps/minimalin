#include <pebble.h>
#include "background_layer.h"
#include "macros.h"

static Layer * s_background_layer;

static void background_layer_update_callback(Layer *layer, GContext *ctx) {
  GRect rect = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, rect, 0, GCornerNone); 
}

void init_background_layer(Layer * root_layer){
  GRect screen_bounds = layer_get_bounds(root_layer);
  s_background_layer = layer_create(screen_bounds);
  layer_set_update_proc(s_background_layer, background_layer_update_callback);
  layer_add_child(root_layer, s_background_layer);
}

void deinit_background_layer(){
  layer_destroy(s_background_layer);
}


