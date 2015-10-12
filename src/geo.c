#include <pebble.h>
#include "geo.h"

#define _45_DEGREES 0x2000
#define _90_DEGREES 0x4000
#define _135_DEGREES 0x6000
#define _225_DEGREES 0xA000
#define _315_DEGREES 0xE000  
  
static int x_for_point(float angle, int radius, const GPoint * center){
  return sin_lookup(angle) * radius / TRIG_MAX_RATIO + center->x;
}

static int y_for_point(float angle, int radius, const GPoint * center){
  return -cos_lookup(angle) * radius / TRIG_MAX_RATIO + center->y;
}

static GPoint point(float angle, int radius, const GPoint * center){
  return (GPoint) {
    .x = x_for_point(angle, radius, center),
    .y = y_for_point(angle, radius, center)
  };
}  

static int use_cos(float angle){
  return angle < _45_DEGREES || (angle > _135_DEGREES && angle < _225_DEGREES) || angle > _315_DEGREES;
}

static int radius_to_border(float angle, int width, int height){
  int32_t radius;
  if(use_cos(angle)){
    radius = (float)TRIG_MAX_RATIO * height / cos_lookup(angle) / 2;
  }else{
    radius = (float)TRIG_MAX_RATIO * width / sin_lookup(angle) / 2;
  }
  if(radius<0){
    radius = -radius;
  }
  return radius;
}

float angle(int time, int max){
  return TRIG_MAX_ANGLE * time / max;
}
  
Vector tick_vector(float angle, const GRect * screen_bounds){
  GPoint center = grect_center_point(screen_bounds);
  int16_t screen_width = screen_bounds->size.w;
  int16_t screen_height = screen_bounds->size.h;
  GPoint ext;
  int32_t radius_tick_end = radius_to_border(angle, screen_width, screen_height);
  if(use_cos(angle)){
    ext.x = x_for_point(angle, radius_tick_end, &center);
    ext.y = angle < _45_DEGREES || angle > _315_DEGREES ? 0 : screen_height;
  }else{
    ext.y = y_for_point(angle, radius_tick_end, &center);
    ext.x = angle > _135_DEGREES ? 0 : screen_width;      
  }
  int32_t radius_tick_start = radius_tick_end - 6;
  return (Vector){
    .ori = point(angle, radius_tick_start, &center),
    .ext = ext
  };
}
