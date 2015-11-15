#pragma once

#include <pebble.h>

typedef struct {
  GColor minute_hand_color;
  GColor hour_hand_color;
  GColor background_color;
  GColor date_color;
  bool date_displayed;
  bool bluetooth_displayed;
  bool rainbow_mode;
} Config;

GColor config_get_minute_hand_color();
GColor config_get_hour_hand_color();
GColor config_get_background_color();
GColor config_get_date_color();
bool config_is_date_displayed();
bool config_is_bluetooth_displayed();
bool config_is_rainbow_mode();
void init_config();
