#include <pebble.h>

#define BACKGROUND_COLOR GColorBlack

// Hour hand constants
#define HOUR_HAND_COLOR GColorRed
#define HOUR_CIRCLE_RADIUS 5
#define HOUR_HAND_STROKE 6
#define HOUR_HAND_RADIUS 39
//#define HOUR_HAND_STROKE 4
//#define HOUR_HAND_RADIUS 31

// Minute hand constants
#define MINUTE_HAND_COLOR GColorWhite
#define MINUTE_HAND_STROKE 6
#define MINUTE_HAND_RADIUS 52
// #define MINUTE_HAND_STROKE 4
// #define MINUTE_HAND_RADIUS 46

// Ticks constants
#define TICK_COLOR GColorLightGray
#define TICK_STROKE 2
#define TICK_LENGTH 6

// Time display constants
#define DISPLAY_COLOR GColorLightGray

// Pebble specific macros
#define DT(ctx, text, rect) graphics_draw_text(ctx, text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIN_19)), rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL)