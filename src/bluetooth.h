#pragma once

#include <pebble.h>

void init_bluetooth_layer(Layer * root_layer);
void deinit_bluetooth_layer();
void mark_dirty_bluetooth_layer();