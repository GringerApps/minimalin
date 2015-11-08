#pragma once

#include <pebble.h>

#define _45_DEGREES 0x2000
#define _90_DEGREES 0x4000
#define _135_DEGREES 0x6000
#define _225_DEGREES 0xA000
#define _315_DEGREES 0xE000 

typedef struct {
  GPoint ori;
  GPoint ext;
} Vector;
  
float angle(int time, int max);
int radius_to_border(float angle, const GSize * size);
int dx(float angle, int radius);
int dy(float angle, int radius);
int x_plus_dx(const int x, const float angle, const int radius);
int y_plus_dy(const int y, const float angle, const int radius);
int use_cos(float angle);
