#pragma once

#include "pebble.h"

#define FOUR 4

typedef enum { North = 0, South, West, East } Position;
typedef enum { Tail, Low, Normal, High, Head } Priority;
typedef enum { First, Second, Third, Fourth } Index;
typedef struct {
  TextBlock * block;
  Priority priority;
  Position position;
} Quadrant;

typedef struct {
  Quadrant * quadrants[FOUR];
  GPoint center;
  int size;
  int hour_hand_radius;
  int minute_hand_radius;
} Quadrants;

Quadrants * quadrants_create(GPoint center, int hour_hand_radius, int minute_hand_radius);
Quadrants * quadrants_destroy(Quadrants * quadrants);
TextBlock * quadrants_add_text_block(Quadrants * quadrants, Layer * root_layer, GFont font, Priority priority, tm * time);
void quadrants_update(Quadrants * quadrants, tm * time);
void quadrants_ready(Quadrants * quadrants);
