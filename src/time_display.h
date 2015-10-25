#pragma once
#include <pebble.h>

typedef struct {
  int hours;
  int minutes;
  int day;
} Time;

/**
 * Displays the current time.
 */
void display_times(GContext *ctx, const GRect * screen_bounds, const Time * current_time);