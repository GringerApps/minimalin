#include <pebble.h>
#include "geo.h"
#include "common.h"
  
int x_plus_dx(const int x, const float angle, const int radius){
  return x + dx(angle, radius);
}

int y_plus_dy(const int y, const float angle, const int radius){
  return y + dy(angle, radius);
}

int dx(float angle, int radius){
  return (float) sin_lookup(angle) * radius / TRIG_MAX_RATIO + 0.5;
}

int dy(float angle, int radius){
  return (float) -cos_lookup(angle) * radius / TRIG_MAX_RATIO + 0.5;
}

void translate(const float angle, const int radius, GPoint * point){
  point->x = x_plus_dx(point->x, angle, radius);
  point->y = y_plus_dy(point->y, angle, radius);
}

int use_cos(float angle){
  return angle < _45_DEGREES || (angle > _135_DEGREES && angle < _225_DEGREES) || angle > _315_DEGREES;
}

int radius_to_border(float angle, const GSize * size){
  float radius;
  if(use_cos(angle)){
    radius = (float)TRIG_MAX_RATIO * size->h / cos_lookup(angle) / 2;
  }else{
    radius = (float)TRIG_MAX_RATIO * size->w / sin_lookup(angle) / 2;
  }
  if(radius<0){
    radius = -radius;
  }
  return radius + 0.5;
}

float angle(int time, int max){
  if(time == 0){
    return 0;
  }
  if(time == max){
    return TRIG_MAX_ANGLE; 
  }
  return TRIG_MAX_ANGLE * time / max;
}
