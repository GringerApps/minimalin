#include <pebble.h>
#include "geo.h"
#include "macros.h"

#define _45_DEGREES 0x2000
#define _90_DEGREES 0x4000
#define _135_DEGREES 0x6000
#define _225_DEGREES 0xA000
#define _315_DEGREES 0xE000  
  
int x_plus_dx(const int x, const float angle, const int radius){
  return x + dx(angle, radius);
}

int y_plus_dy(const int y, const float angle, const int radius){
  return y + dy(angle, radius);
}

int dx(float angle, int radius){
  return sin_lookup(angle) * radius / TRIG_MAX_RATIO;
}

int dy(float angle, int radius){
  return -cos_lookup(angle) * radius / TRIG_MAX_RATIO;
}

void translate(const float angle, const int radius, GPoint * point){
  point->x = x_plus_dx(point->x, angle, radius);
  point->y = y_plus_dy(point->y, angle, radius);
}

static int use_cos(float angle){
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
  return TRIG_MAX_ANGLE * time / max;
}

Vector tick_vector(float angle, const GRect * screen_bounds){
  GPoint center      = grect_center_point(screen_bounds);
  const GSize * size = &screen_bounds->size; 
  GPoint ext;
  int32_t radius_tick_end = radius_to_border(angle, size);
  if(use_cos(angle)){
    ext.x = x_plus_dx(center.x, angle, radius_tick_end),
    ext.y = angle < _45_DEGREES || angle > _315_DEGREES ? 0 : size->h;
  }else{
    ext.y = y_plus_dy(center.y, angle, radius_tick_end),
    ext.x = angle > _135_DEGREES ? 0 : size->w;      
  }
  int32_t radius_tick_start = radius_tick_end - TICK_LENGTH;
  GPoint ori = center;
  translate(angle, radius_tick_start, &ori);
  return (Vector){
    .ori = ori,
    .ext = ext
  };
}
