#pragma once

#include "pebble.h"

typedef struct {
  GPoint head;
  GPoint tail;
} Segment;

#define SEGMENT(head,tail) (Segment){(head),(tail)}

bool intersect(Segment seg, GRect frame);
float angle(int value, int max);
float angle_hour(tm * time, const bool with_delta);
float angle_minute(tm * time);
GPoint gpoint_on_circle(const GPoint center, const int angle, const int radius);
GRect grect_from_center_and_size(GPoint center, GSize size);
