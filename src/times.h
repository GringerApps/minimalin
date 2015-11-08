#pragma once

#include <pebble.h>
#include "common.h"

void init_time_layer(Layer * root_layer);
void deinit_time_layer();
void mark_dirty_time_layer();
