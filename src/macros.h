#pragma once

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

// Date constants
#define DATE_RADIUS 28

// Ticks constants
#define TICK_COLOR GColorLightGray
#define TICK_STROKE 2
#define TICK_LENGTH 6

// Time display constants
#define TIME_COLOR GColorLightGray
#define DATE_COLOR GColorDarkGray

// Pebble specific macros
#define SAA(ctx, boolean) graphics_context_set_antialiased(ctx, boolean)
#define SSW(ctx, stroke_width) graphics_context_set_stroke_width(ctx, stroke_width)
#define SSC(ctx, color)  graphics_context_set_stroke_color(ctx, color)
#define SFC(ctx, color)  graphics_context_set_fill_color(ctx, color)
#define DL(clx, p1, p2)  graphics_draw_line(ctx, p1, p2)
#define FC(ctx, center, radius)  graphics_fill_circle(ctx, center, radius)
#define DT(ctx, text, rect) graphics_draw_text(ctx, text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NUPE_23)), rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL)