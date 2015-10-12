#pragma once
#include <pebble.h>

typedef struct {
  GPoint ori;
  GPoint ext;
} Vector;
  
float angle(int time, int max);
Vector tick_vector(float angle, const GRect * screen_bounds);
