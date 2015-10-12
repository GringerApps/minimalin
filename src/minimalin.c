#include <pebble.h>
#include "geo.h"
  
#define HAND_STROKE 4
#define CENTER_CIRCLE_RADIUS 4
#define HOUR_HAND_RADIUS 31
#define MINUTE_HAND_RADIUS 46
#define BACKGROUND_COLOR GColorBlack
#define HOUR_HAND_COLOR GColorRed
#define MINUTE_HAND_COLOR GColorWhite
#define TIME_INFO_COLOR GColorWhite
#define TICK_COLOR GColorWhite
#define TICK_STROKE 2

#define SAA(ctx, boolean) graphics_context_set_antialiased(ctx, boolean)
#define SSW(ctx, stroke_width) graphics_context_set_stroke_width(ctx, stroke_width)
#define SSC(ctx, color)  graphics_context_set_stroke_color(ctx, color)
#define SFC(ctx, color)  graphics_context_set_fill_color(ctx, color)
#define DL(clx, p1, p2)  graphics_draw_line(ctx, p1, p2)
#define FC(ctx, center, radius)  graphics_fill_circle(ctx, center, radius);
  
typedef struct {
  int hours;
  int minutes;
} Time;


static Window *s_main_window;
static Layer *s_time_layer;
static Time current_time;
static GRect screen_bounds;
static GPoint screen_center;

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int hours = tick_time->tm_hour;
  if(hours > 12){
    hours -= 12;
  }else if(hours == 0){
    hours = 12;
  }
  current_time.hours   = hours;
  current_time.minutes = tick_time->tm_min;
  if(s_time_layer) {
    layer_mark_dirty(s_time_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static GPoint point_for_angle_and_radius(float angle, int radius){
  return (GPoint) {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)radius / TRIG_MAX_RATIO) + screen_center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)radius / TRIG_MAX_RATIO) + screen_center.y,
  };
}

static void draw_hands(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  
  
  float minute_angle = angle(current_time.minutes, 60);
  float hour_angle = angle(current_time.hours, 12);
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  GPoint minute_hand = point_for_angle_and_radius(minute_angle, MINUTE_HAND_RADIUS);
  GPoint hour_hand = point_for_angle_and_radius(hour_angle, HOUR_HAND_RADIUS);
  SAA(ctx, true);
  SSW(ctx, HAND_STROKE);
  SSC(ctx, MINUTE_HAND_COLOR);
  DL(ctx, screen_center, minute_hand);
  SSC(ctx, HOUR_HAND_COLOR);
  DL(ctx, screen_center, hour_hand);
  SFC(ctx, HOUR_HAND_COLOR);
  FC(ctx, screen_center, CENTER_CIRCLE_RADIUS);
}

static void draw_closest_time(Layer *layer, GContext *ctx) {
  int hours = current_time.hours;
  float hour_angle = angle(hours, 12);
  GPoint hour_number = point_for_angle_and_radius(hour_angle, MINUTE_HAND_RADIUS + 10); 
  hour_number.x -= 10;
  hour_number.y -= 10;    
  static char buffer[] = "00";
  snprintf(buffer, sizeof("00"), "%d", hours);
  GRect rect = GRect(hour_number.x, hour_number.y, 20, 20);
  graphics_context_set_text_color(ctx, TIME_INFO_COLOR);
  graphics_draw_text(
    ctx, 
    buffer, 
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIN_19)),
    rect,
    GTextOverflowModeWordWrap, 
    GTextAlignmentCenter,
    NULL
  );
}

static void draw_background(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, screen_bounds, 0, GCornerNone);
}

static void draw_closest_tick(Layer *layer, GContext *ctx) {
  float hour_tick_angle = angle(current_time.hours, 12);
  Vector vector = tick_vector(hour_tick_angle, &screen_bounds);
  SSC(ctx, TICK_COLOR);
  SSW(ctx, TICK_STROKE);
  DL(ctx, vector.ori, vector.ext);
  float minute_tick_angle = angle(current_time.minutes / 5, 12);
  if(minute_tick_angle == hour_tick_angle){
    return;
  }
  vector = tick_vector(minute_tick_angle, &screen_bounds);
  SSC(ctx, TICK_COLOR);
  SSW(ctx, TICK_STROKE);
  DL(ctx, vector.ori, vector.ext);
}

static void time_layer_update_callback(Layer *layer, GContext *ctx) {
  draw_background(layer, ctx);
  draw_hands(layer, ctx);
  draw_closest_time(layer, ctx);
  draw_closest_tick(layer, ctx);
}

static void main_window_load(Window *window) {
  Layer * root_layer = window_get_root_layer(window);
  screen_bounds = layer_get_bounds(root_layer);
  screen_center = grect_center_point(&screen_bounds);
  s_time_layer = layer_create(screen_bounds);
  layer_set_update_proc(s_time_layer, time_layer_update_callback);
  layer_add_child(root_layer, s_time_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_time_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
