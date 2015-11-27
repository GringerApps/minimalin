#pragma once

#include <pebble.h>

typedef struct {
  int hour;
  int minute;
  int day;
} Time;

// Pebble specific macros
#define set_stroke_width(ctx, stroke_width) graphics_context_set_stroke_width(ctx, stroke_width)
#define set_stroke_color(ctx, color)  graphics_context_set_stroke_color(ctx, color)
#define set_fill_color(ctx, color)  graphics_context_set_fill_color(ctx, color)
#define draw_line(clx, p1, p2)  graphics_draw_line(ctx, p1, p2)
#define fill_circle(ctx, center, radius)  graphics_fill_circle(ctx, center, radius
#define set_text_color(ctx, color)  graphics_context_set_text_color(ctx, color)
#define draw_text(ctx, text, font, rect) graphics_draw_text(ctx, text, font, rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL)

GRect grect_translated(const GRect rect, const int x, const int y);
GPoint gpoint_on_circle(const GPoint center, const int angle, const int radius);
Time get_current_time();
void update_current_time();
void init_font();
void deinit_font();
GFont get_font();
