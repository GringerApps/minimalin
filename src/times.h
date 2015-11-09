#pragma once

#include <pebble.h>
#include "common.h"

void init_times(Layer * root_layer);
void deinit_times();
void mark_dirty_time_layer();
