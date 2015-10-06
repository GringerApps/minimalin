#include <pebble.h>

#define RADIUS 55
#define HAND_MARGIN 10
  
typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_time_layer;
static Time current_time;
static GRect screen_bounds;

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  current_time.hours = tick_time->tm_hour;
  current_time.minutes = tick_time->tm_min;
  if(s_time_layer) {
    layer_mark_dirty(s_time_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void draw_hands(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  
  GPoint center = grect_center_point(&screen_bounds);
  float minute_angle = TRIG_MAX_ANGLE * current_time.minutes / 60;
  float hour_angle = TRIG_MAX_ANGLE * current_time.hours / 12;
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * current_time.minutes / 60) * (int32_t)(RADIUS - HAND_MARGIN) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * current_time.minutes / 60) * (int32_t)(RADIUS - HAND_MARGIN) / TRIG_MAX_RATIO) + center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(RADIUS - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(RADIUS - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + center.y,
  };

  graphics_draw_line(ctx, center, hour_hand);
  graphics_draw_line(ctx, center, minute_hand);
}

static void time_layer_update_callback(Layer *layer, GContext *ctx) {
  draw_hands(layer, ctx);  
}

static void main_window_load(Window *window) {
  Layer * root_layer = window_get_root_layer(window);
  screen_bounds = layer_get_bounds(root_layer);
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