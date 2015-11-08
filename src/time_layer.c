#include <pebble.h>
#include "time_layer.h"
#include "geo.h"
#include "config.h"
#include "common.h"

#define DATE_RADIUS 28
#define TIME_COLOR GColorLightGray
#define DATE_COLOR GColorDarkGray
#define MARGIN 6
#define CHAR_HEIGHT 23
#define CHAR_WIDTH 23
#define VERTICAL_TOP_DIGITS_OFFSET 2
#define VERTICAL_BOTTOM_DIGITS_OFFSET 1
#define LETTER_OFFSET -3

typedef enum { NoLeading = 1, LeadingZero = 2, Analog = 5 } TimeFormat;

static Layer * s_time_layer;

static bool conflicting_times(const int hour, const int minute){
  return (hour == 12 && minute < 5) || hour == minute / 5;
}

static bool display_vertical(const int hour){
  return (hour > 1 && hour < 5) || (hour > 7 && hour < 11);
}

static void set_size_for_format(const TimeFormat format, GSize * size){
  size->w = (format + 1)/ 2 * CHAR_WIDTH;
  size->h = CHAR_HEIGHT;
}

static int box_origin_x(const int x, const GSize * box_size){
  return x - box_size->w / 2;
}

static int box_origin_y(const int y, const GSize * box_size){
  return y - box_size->h / 2 + LETTER_OFFSET;
}

static void set_display_box(const float angle, const TimeFormat format, const GRect * screen_bounds, GRect * display_box){
  const GSize * screen_size = &screen_bounds->size;
  GPoint center             = grect_center_point(screen_bounds);
  GSize * size              = &display_box->size;
  set_size_for_format(format, size);
  int display_margin = radius_to_border(angle, size);
  if((int)angle % _90_DEGREES != 0){
    display_margin += 2;
  }
  int radius         = radius_to_border(angle, screen_size) - MARGIN - display_margin;
  display_box->origin.x = box_origin_x(x_plus_dx(center.x, angle, radius), size);
  display_box->origin.y = box_origin_y(y_plus_dy(center.y, angle, radius), size);
}

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
  draw_text(ctx, buffer, get_font(), *rect);
}

static void display_vertical_time(GContext * ctx, const GRect * screen_bounds, const Time * current_time){
  int hour         = current_time->hour;
  float time_angle = angle(hour, 12);
  GRect rect;
  set_display_box(time_angle, LeadingZero, screen_bounds, &rect);
  rect.origin.y -= rect.size.h / 2 + VERTICAL_TOP_DIGITS_OFFSET;
  display_time(ctx, &rect, NoLeading, hour);
  set_display_box(time_angle, LeadingZero, screen_bounds, &rect);
  rect.origin.y += rect.size.h / 2 + VERTICAL_BOTTOM_DIGITS_OFFSET;
  display_time(ctx, &rect, LeadingZero, current_time->minute);
}

static void display_horizontal_time(GContext * ctx, const GRect * screen_bounds, const Time * current_time){
  int hour         = current_time->hour;
  float time_angle = angle(hour, 12);
  GRect rect;
  set_display_box(time_angle, Analog, screen_bounds, &rect);
  display_time(ctx, &rect, Analog, hour * 100 + current_time->minute);
}

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

static void time_layer_update_callback(Layer * layer, GContext *ctx){
  GRect screen_bounds = layer_get_bounds(layer);
  Time current_time = get_current_time();
  graphics_context_set_text_color(ctx, TIME_COLOR);
  int hour = current_time.hour;
  if(conflicting_times(hour, current_time.minute)){
    if (display_vertical(hour)){
      display_vertical_time(ctx, &screen_bounds, &current_time);
    }else{
      display_horizontal_time(ctx, &screen_bounds, &current_time);
    }
  }else{
    display_normal_time(ctx, &screen_bounds, NoLeading, current_time.hour, current_time.hour);
    display_normal_time(ctx, &screen_bounds, LeadingZero, current_time.minute / 5, current_time.minute);
  }
  if(config_is_date_displayed()){
    display_date(ctx, &screen_bounds, current_time.day);
  }
}

void init_time_layer(Layer * root_layer){
  GRect screen_bounds = layer_get_bounds(root_layer);
  s_time_layer = layer_create(screen_bounds);
  layer_set_update_proc(s_time_layer, time_layer_update_callback);
  layer_add_child(root_layer, s_time_layer);
}

void deinit_time_layer(){
  layer_destroy(s_time_layer); 
}

void mark_dirty_time_layer(){
  if(s_time_layer){
    layer_mark_dirty(s_time_layer);
  }
}
