#pragma once

#include <pebble.h>

typedef struct {
  int hour;
  int minute;
  int day;
} Time;

Time get_current_time();
void update_current_time();
