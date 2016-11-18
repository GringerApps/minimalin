#pragma once

#include <pebble.h>


#define NOW 0

#ifdef PBL_PLATFORM_EMERY
#define RAINBOW_HAND_OFFSET_X 5
#define RAINBOW_HAND_OFFSET_Y 81
#define TIME_CONFLICT_OFFSET 10
#define TICK_WIDTH 2
#define HOUR_HAND_WIDTH 8
#define MINUTE_HAND_WIDTH 8
#define CENTER_CIRCLE_RADIUS 7
#define MINUTE_HAND_RADIUS 78
#define HOUR_HAND_RADIUS 58
static GPoint ticks_points[12][2] = {
  {{100, 0}, {100, 10}},
  {{167, 0}, {163, 10}},
  {{200, 57}, {190, 62}},
  {{200, 114}, {190, 114}},
  {{200, 171}, {190, 166}},
  {{167, 228}, {163, 219}},
  {{100, 228}, {100, 219}},
  {{33, 228}, {38, 219}},
  {{0, 171}, {10, 166}},
  {{0, 114}, {10, 114}},
  {{0, 57}, {10, 62}},
  {{33, 0}, {38, 10}}
};
static GPoint time_points[12] = {
  {100, 21},
  {156, 21},
  {177, 65},
  {177, 112},
  {177, 160},
  {156, 198},
  {100, 198},
  {44, 198},
  {23, 160},
  {23, 112},
  {23, 65},
  {44, 21}
};
#elif PBL_ROUND
#define RAINBOW_HAND_OFFSET_X 5
#define RAINBOW_HAND_OFFSET_Y 55
#define TIME_CONFLICT_OFFSET 10
#define TICK_WIDTH 2
#define HOUR_HAND_WIDTH 6
#define MINUTE_HAND_WIDTH 6
#define CENTER_CIRCLE_RADIUS 5
#define MINUTE_HAND_RADIUS 52
#define HOUR_HAND_RADIUS 39
static GPoint ticks_points[12][2] = {
  {{90, 0}  , {90, 6}  },
  {{135,12} , {132,18}  },
  {{168,45} , {162,48} },
  {{180,90} , {174,90} },
  {{168,135}, {162,132}},
  {{135,168}, {132,162}},
  {{90, 180}, {90, 174}},
  {{45, 168}, {48, 162}},
  {{12, 135}, {18, 132}},
  {{0,  90} , {6,  90} },
  {{12, 45} , {18, 48} },
  {{45, 12} , {48, 18}  }
};
static GPoint time_points[12] = {
  {90,  17} ,
  {124, 28} ,
  {150, 50} ,
  {161, 86} ,
  {148, 124},
  {124, 146},
  {90,  159},
  {54,  147},
  {29,  124},
  {18,  87} ,
  {30,  52} ,
  {54,  28} ,
};
#else
#define RAINBOW_HAND_OFFSET_X 5
#define RAINBOW_HAND_OFFSET_Y 55
#define TIME_CONFLICT_OFFSET 10
#define TICK_WIDTH 2
#define HOUR_HAND_WIDTH 6
#define MINUTE_HAND_WIDTH 6
#define CENTER_CIRCLE_RADIUS 5
#define MINUTE_HAND_RADIUS 52
#define HOUR_HAND_RADIUS 39
static GPoint ticks_points[12][2] = {
  {{72, 0}  , {72, 7}  },
  {{120,0}  , {117,7}  },
  {{144,42} , {137,46} },
  {{144,84} , {137,84} },
  {{144,126}, {137,122}},
  {{120,168}, {117,161}},
  {{72, 168}, {72, 161}},
  {{24, 168}, {27, 161}},
  {{0,  126}, {7,  122}},
  {{0,  84} , {7,  84} },
  {{0,  42} , {7,  46} },
  {{24, 0}  , {27, 7}  }
};
static GPoint time_points[12] = {
  {72,  15} ,
  {112, 15} ,
  {126, 47} ,
  {126, 82},
  {126, 117},
  {112, 145} ,
  {72,  145},
  {32,  145},
  {18,  117},
  {18,  82} ,
  {18,  47} ,
  {32,  15} ,
};
#endif
