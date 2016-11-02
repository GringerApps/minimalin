#pragma once

#include "pebble.h"

#define TEXT_BLOCK_SIZE GSize(70, 23)

typedef struct {
  Layer * layer;
  GFont font;
  GRect frame;
  GColor color;
  bool enabled;
  bool ready;
  char text[20];
} TextBlock;

TextBlock * text_block_create(Layer * parent_layer, const GPoint center, const GFont font);
TextBlock * text_block_destroy(TextBlock * text_block);
void text_block_set_text(TextBlock * text_block, const char * text, const GColor color);
void text_block_set_visible(TextBlock * text_block, const bool visible);
bool text_block_get_visible(TextBlock * text_block);
void text_block_set_enabled(TextBlock * text_block, const bool enable);
bool text_block_get_enabled(TextBlock * text_block);
void text_block_set_ready(TextBlock * text_block, const bool enable);
bool text_block_get_ready(TextBlock * text_block);
void text_block_move(TextBlock * text_block, const GPoint center);
