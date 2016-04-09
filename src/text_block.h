#include <pebble.h>

typedef struct {
  Layer * layer;
  GFont font;
  GRect frame;
  GColor color;
  char text[20];
} TextBlock;

TextBlock * text_block_create(Layer * parent_layer, const GPoint center, const GFont font);
TextBlock * text_block_destroy(TextBlock * text_block);
void text_block_set_text(TextBlock * text_block, const char * text, const GColor color);
void text_block_set_visible(TextBlock * text_block, const bool visible);
void text_block_move(TextBlock * text_block, const GPoint center);
