#include <pebble.h>
#include "text_block.h"

static void text_block_update_proc(struct Layer *layer, GContext *ctx){
  TextBlock ** data = (TextBlock**) layer_get_data(layer);
  TextBlock * text_block = *data;
  graphics_context_set_text_color(ctx, text_block->color);
  graphics_draw_text(ctx, text_block->text, text_block->font, text_block->frame, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

TextBlock * text_block_create(Layer * parent_layer, const GPoint center, const GFont font){
  TextBlock * text_block = (TextBlock *) malloc(sizeof(TextBlock));
  Layer * layer = layer_create_with_data(layer_get_frame(parent_layer), sizeof(TextBlock*));
  TextBlock ** data = (TextBlock**) layer_get_data(layer);
  *data = text_block;
  text_block->layer = layer;
  text_block->font = font;
  const GSize size = GSize(100, 23);
  const GRect frame = (GRect) {
    .origin = GPoint(center.x - size.w / 2 , center.y - size.h / 2),
    .size   = size
  };
  text_block->frame = frame;
  layer_set_update_proc(layer, text_block_update_proc);
  layer_add_child(parent_layer, layer);
  return text_block;
}

TextBlock * text_block_destroy(TextBlock * text_block){
  layer_destroy(text_block->layer);
  free(text_block);
  return NULL;
}

void text_block_set_text(TextBlock * text_block, const char * text, const GColor color){
  strncpy(text_block->text, text, sizeof(text_block->text));
  text_block->color = color;
  layer_mark_dirty(text_block->layer);
}

void text_block_set_visible(TextBlock * text_block, const bool visible){
  layer_set_hidden(text_block->layer, !visible);
}

void text_block_move(TextBlock * text_block, const GPoint center){
  const GSize size = GSize(100, 23);
  const GRect frame = (GRect) {
    .origin = GPoint(center.x - size.w / 2 , center.y - size.h / 2),
    .size   = size
  };
  text_block->frame = frame;
  layer_mark_dirty(text_block->layer);
}
