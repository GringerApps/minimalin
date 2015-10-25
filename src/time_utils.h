#pragma once

#include <pebble.h>

typedef struct {
  int hours;
  int minutes;
  int day;
} Time;

void set_current_time(Time * current_time);
void update_current_time();