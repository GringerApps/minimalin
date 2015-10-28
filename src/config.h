#pragma once

#include <pebble.h>

typedef struct {
  GColor minute_hand_color;
  GColor hour_hand_color;
  bool date_displayed;
  bool bluetooth_displayed;
} Config;

GColor config_get_minute_hand_color();
GColor config_get_hour_hand_color();
bool config_is_date_displayed();
bool config_is_bluetooth_displayed();
void init_config();