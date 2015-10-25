#include <pebble.h>
#include "time_display.h"
#include "geo.h"
#include "macros.h"

#define CHAR_HEIGHT 23
#define CHAR_WIDTH 23

typedef enum { NoLeading = 1, LeadingZero = 2, Analog = 5 } TimeFormat;

/**
 * Returns whether the hour and minute displays would conflict.
 *
 */
static bool conflicting_times(const int hour, const int minute){
  return (hour == 12 && minute < 5) || hour == minute / 5;
}

/**
 * Returns whether a conflicting time should be displayed as vertical or horizontal.
 */
static bool display_vertical(const int hour){
  return (hour > 1 && hour < 5) || (hour > 7 && hour < 11);
}

/**
 * Sets the GSize of the box necessary to display a time in the given format.
 */
static void set_size_for_format(const TimeFormat format, GSize * size){
  size->w = (format + 1)/ 2 * CHAR_WIDTH;
  size->h = CHAR_HEIGHT;
}

static int box_origin_x(const int x, const GSize * box_size){
  return x - box_size->w / 2 + 1; // TODO: fix +1 due to font
}

static int box_origin_y(const int y, const GSize * box_size){
  return y - box_size->h / 2 - 3; // TODO: fix -4 due to font
}

/**
 * Sets the GRect of the box necessary to display a time in the given format at the given angle.
 */
static void set_display_box(const float angle, const TimeFormat format, const GRect * screen_bounds, GRect * display_box){
  const GSize * screen_size = &screen_bounds->size;
  GPoint center             = grect_center_point(screen_bounds);
  GSize * size              = &display_box->size;
  set_size_for_format(format, size);
  int display_margin = radius_to_border(angle, size); // TODO: refactor radius_to_border to use GSize
  if((int)angle % _90_DEGREES != 0){
    display_margin += 2;
  }
  int radius         = radius_to_border(angle, screen_size) - TICK_LENGTH - display_margin;
  display_box->origin.x = box_origin_x(x_plus_dx(center.x, angle, radius), size);
  display_box->origin.y = box_origin_y(y_plus_dy(center.y, angle, radius), size);
}

/**
 * Displays the given time in the given format in the given GRect.
 */
static void display_time(GContext * ctx, const GRect * rect, const TimeFormat time_format, const int time)
{
  char buffer[] = "00:00";
  if(time_format == NoLeading){
    snprintf(buffer, sizeof(buffer), "%d", time);
  }else if(time_format == LeadingZero){
    snprintf(buffer, sizeof(buffer), "%02d", time);
  }else{
    snprintf(buffer, sizeof(buffer), "%02d:%02d", time / 100, time % 100);
  }
  DT(ctx, buffer, *rect);
}

/**
 * Displays the given time vertically.
 */
static void display_vertical_time(GContext * ctx, const GRect * screen_bounds, const Time * current_time){
  int hour         = current_time->hours;
  float time_angle = angle(hour, 12);
  GRect rect;
  set_display_box(time_angle, LeadingZero, screen_bounds, &rect);
  rect.origin.y -= rect.size.h / 2;
  display_time(ctx, &rect, NoLeading, hour);
  rect.origin.y += rect.size.h;
  display_time(ctx, &rect, LeadingZero, current_time->minutes);
}

/**
 * Displays the given time horizontally.
 */
static void display_horizontal_time(GContext * ctx, const GRect * screen_bounds, const Time * current_time){
  int hour         = current_time->hours;
  float time_angle = angle(hour, 12);
  GRect rect;
  set_display_box(time_angle, Analog, screen_bounds, &rect);
  display_time(ctx, &rect, Analog, hour * 100 + current_time->minutes);
}

/**
 * Displays a one or two digit time for the given tick number with the given time in the given format.
 */
static void display_normal_time(GContext * ctx, const GRect * screen_bounds, const TimeFormat format, const int tick_number, const int time){
  float time_angle  = angle(tick_number, 12);
  GRect rect;
  set_display_box(time_angle, format, screen_bounds, &rect);
  display_time(ctx, &rect, format, time);
}

static void display_date(GContext * ctx, const GRect * screen_bounds, const int day){
  graphics_context_set_text_color(ctx, DATE_COLOR);
  GRect rect;
  GPoint center = grect_center_point(screen_bounds);
  set_size_for_format(NoLeading, &rect.size);
  rect.origin.x = box_origin_x(center.x, &rect.size);
  rect.origin.y = box_origin_y(center.y + DATE_RADIUS, &rect.size);
  display_time(ctx, &rect, NoLeading, day);
}

void display_times(GContext *ctx, const GRect * screen_bounds, const Time * current_time) {
  graphics_context_set_text_color(ctx, TIME_COLOR);
  int hour = current_time->hours;
  if(conflicting_times(hour, current_time->minutes)){
    if (display_vertical(hour)){
      display_vertical_time(ctx, screen_bounds, current_time);
    }else{
      display_horizontal_time(ctx, screen_bounds, current_time);
    }
  }else{
    display_normal_time(ctx, screen_bounds, NoLeading, current_time->hours, current_time->hours);
    display_normal_time(ctx, screen_bounds, LeadingZero, current_time->minutes / 5, current_time->minutes);
  }
  display_date(ctx, screen_bounds, current_time->day);
}