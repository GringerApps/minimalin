#pragma once

#include <pebble.h>

void init_hands(Layer * root_layer);
void deinit_hands();
void hands_update_time_changed();
void hands_update_minute_hand_config_changed();
void hands_update_hour_hand_config_changed();
void hands_update_rainbow_mode_config_changed();
