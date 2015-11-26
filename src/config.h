#pragma once

#include <pebble.h>

typedef void (*ConfigUpdatedCallback)();
typedef enum { NoIcon = 0, Bluetooth = 1, Heart = 2 } BluetoothIcon;

GColor config_get_minute_hand_color();
GColor config_get_hour_hand_color();
GColor config_get_background_color();
GColor config_get_date_color();
GColor config_get_time_color();
bool config_is_date_displayed();
BluetoothIcon config_get_bluetooth_icon();
bool config_is_rainbow_mode();
void init_config(ConfigUpdatedCallback);
