#pragma once

#include <pebble.h>

void init_tick_layer(Layer * root_layer);
void deinit_tick_layer();
void mark_dirty_tick_layer();