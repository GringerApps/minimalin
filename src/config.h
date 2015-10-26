#pragma once

#include <pebble.h>

typedef struct {
  GColor minute_hand_color;
} Config;

GColor config_get_minute_hand_color();
void init_config();