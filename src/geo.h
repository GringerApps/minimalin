#pragma once
#include <pebble.h>

typedef struct {
  GPoint ori;
  GPoint ext;
} Vector;
  
float angle(int time, int max);
Vector tick_vector(float angle, const GRect * screen_bounds);
int radius_to_border(float angle, const GSize * size);
int dx(float angle, int radius);
int dy(float angle, int radius);
int x_plus_dx(const int x, const float angle, const int radius);
int y_plus_dy(const int y, const float angle, const int radius);
void translate(const float angle, const int radius, GPoint * point);