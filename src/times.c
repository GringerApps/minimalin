#include <pebble.h>
#include "times.h"
#include "config.h"
#include "common.h"

#define DATE_Y_OFFSET 28

typedef enum { Hour, Minute } TimeType;

#ifdef PBL_ROUND
static GPoint time_points[12] = {
  {90,  21} ,
  {124, 30} ,
  {150, 56} ,
  {159, 90} ,
  {150, 124},
  {124, 150},
  {90,  159},
  {56,  150},
  {30,  124},
  {21,  90} ,
  {30,  56} ,
  {56,  30} ,
};
#else
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
static GRect s_bounds;
static GPoint s_center;
static Layer * s_time_layer;

static bool times_overlap(const int hour, const int minute){
  return (hour == 12 && minute < 5) || hour == minute / 5;
}

static bool time_displayed_horizontally(const int hour, const int minute){
  return times_overlap(hour, minute) && (hour <= 1 || hour >= 11 || (hour <= 7 && hour >= 5));
}

static bool time_displayed_vertically(const int hour, const int minute){
  return times_overlap(hour, minute) && ((hour > 1 && hour < 5) || (hour > 7 && hour < 11));
}

static GSize get_display_box_size(const char * text){
  return graphics_text_layout_get_content_size(text, get_font(), s_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
}

static GRect get_display_box(const GPoint box_center, const char * time){
  const GSize box_size    = get_display_box_size(time);
  const GPoint box_origin = GPoint(box_center.x - box_size.w / 2, box_center.y - box_size.h / 2);
  const GRect box         = (GRect) { .origin = box_origin, .size = box_size };
  return box;
}

static GPoint get_time_point(const int time, const TimeType type){
  if(type == Minute){
    return time_points[time / 5];
  }
  return time_points[time % 12];
}

static void display_number(GContext * ctx, const GRect box, const int number, const bool leading_zero){
  char buffer[] = "00";
  if(leading_zero){
    snprintf(buffer, sizeof(buffer), "%02d", number);
  }else{
    snprintf(buffer, sizeof(buffer), "%d", number);
  }
  draw_text(ctx, buffer, get_font(), box);  
}

static void display_time(GContext * ctx, const int hour, const int minute){
  graphics_context_set_text_color(ctx, config_get_time_color());
  char buffer[] = "00:00";
  if(time_displayed_horizontally(hour, minute)){
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    const GPoint box_center = get_time_point(hour, Hour);
    const GRect box         = get_display_box(box_center, "00:00");
    draw_text(ctx, buffer, get_font(), box);
  }else{
    GRect hour_box;
    GRect minute_box;
    if(time_displayed_vertically(hour, minute)){
      const GPoint box_center = get_time_point(hour, Hour);
      const GRect box         = get_display_box(box_center, "00");
      hour_box                = grect_translated(box, 0, - box.size.h / 2 - 4);
      minute_box              = grect_translated(box, 0, box.size.h / 2 - 2);
    }else{
      const GPoint hour_box_center   = get_time_point(hour, Hour);
      hour_box                       = get_display_box(hour_box_center, "00");
      const GPoint minute_box_center = get_time_point(minute, Minute);
      minute_box                     = get_display_box(minute_box_center, "00");
    }
    display_number(ctx, hour_box, hour, false);
    display_number(ctx, minute_box, minute, true);
  }
}

static void display_date(GContext * ctx, const int day){
  set_text_color(ctx, config_get_date_color());
  const GRect box = get_display_box(s_center, "00");
  display_number(ctx, grect_translated(box, 0, DATE_Y_OFFSET), day, false);
}

static void time_layer_update_callback(Layer * layer, GContext *ctx){
  Time current_time = get_current_time();
  display_time(ctx, current_time.hour, current_time.minute);
  if(config_is_date_displayed()){
    display_date(ctx, current_time.day);
  }
}

void init_times(Layer * root_layer){
  s_bounds = layer_get_bounds(root_layer);
  s_center = grect_center_point(&s_bounds);
  s_time_layer = layer_create(s_bounds);
  layer_set_update_proc(s_time_layer, time_layer_update_callback);
  layer_add_child(root_layer, s_time_layer);
}

void deinit_times(){
  layer_destroy(s_time_layer); 
}

void mark_dirty_time_layer(){
  if(s_time_layer){
    layer_mark_dirty(s_time_layer);
  }
}
