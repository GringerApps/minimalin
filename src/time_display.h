#pragma once
#include <pebble.h>

typedef struct {
  int hours;
  int minutes;
} Time;

/**
 * Displays the current time.
 */
void display_times(GContext *ctx, const GRect * screen_bounds, const Time * current_time);