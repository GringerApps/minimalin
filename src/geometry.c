#include "geometry.h"

bool intersect(const Segment seg, const GRect frame){
  const GPoint origin = frame.origin;
  const GSize size = frame.size;
  const int x_min = origin.x;
  const int y_min = origin.y;
  const int x_max = origin.x + size.w;
  const int y_max = origin.y + size.h;
  const GPoint head = seg.head;
  const GPoint tail = seg.tail;
  if( (head.x < x_min && tail.x < x_min) || (head.y < y_min && tail.y < y_min) ||
      (head.x > x_max && tail.x > x_max) || (head.y > y_max && tail.y > y_max)){
    return false;
  }
  const float coef = (float)(tail.y - head.y) / (tail.x - head.x);
  float y = coef * (x_min - head.x) + head.y;
  if (y > y_min && y < y_max){
    return true;
  }
  y = coef * (x_max - head.x) + head.y;
  if (y > y_min && y < y_max){
    return true;
  }
  float x = (y_min - head.y) / coef + head.x;
  if (x > x_min && x < x_max){
    return true;
  }
  x = (y_max - head.y) / coef + head.x;
  if (x > x_min && x < x_max){
    return true;
  }
  return false;
}

float angle(const int value, const int max){
  if(value == 0 || value == max)
    return 0;
  return TRIG_MAX_ANGLE * value / max;
}

float angle_hour(const tm * const time, const bool with_delta){
  const int hour = time->tm_hour % 12;
  if(with_delta){
    return angle(hour * 50 + time->tm_min * 50 / 60, 600);
  }
  return angle(hour, 12);
}

float angle_minute(const tm * const time){
  return angle(time->tm_min, 60);
}

GPoint gpoint_on_circle(const GPoint center, const int angle, const int radius){
  const int diameter = radius * 2;
  const GRect grect_for_polar = GRect(center.x - radius + 1, center.y - radius + 1, diameter, diameter);
  return gpoint_from_polar(grect_for_polar, GOvalScaleModeFitCircle, angle);
}

GRect grect_from_center_and_size(const GPoint center, const GSize size){
  return (GRect) {
    .origin = GPoint(center.x - size.w / 2 , center.y - size.h / 2),
    .size   = size
  };
}
