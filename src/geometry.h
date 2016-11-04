#pragma once

#include "pebble.h"

typedef struct {
  GPoint head;
  GPoint tail;
} Segment;

#define SEGMENT(head,tail) (Segment){(head),(tail)}

bool intersect(const Segment seg, const GRect frame);
float angle(const int value, const int max);
float angle_hour(const tm * const time, const bool with_delta);
float angle_minute(const tm * const time);
GPoint gpoint_on_circle(const GPoint center, const int angle, const int radius);
GRect grect_from_center_and_size(const GPoint center, const GSize size);
