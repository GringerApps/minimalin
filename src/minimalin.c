#include <pebble.h>
#include "common.h"

#define HOUR_HAND_COLOR GColorRed
#define d(string, ...) APP_LOG (APP_LOG_LEVEL_DEBUG, string, ##__VA_ARGS__)
#define e(string, ...) APP_LOG (APP_LOG_LEVEL_ERROR, string, ##__VA_ARGS__)
#define i(string, ...) APP_LOG (APP_LOG_LEVEL_INFO, string, ##__VA_ARGS__)

#ifdef PBL_ROUND
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

typedef void (*ConfigUpdatedCallback)();

typedef enum { NoIcon = 0, Bluetooth = 1, Heart = 2 } BluetoothIcon;

typedef enum { Celsius = 0, Fahrenheit= 1 } TemperatureUnit;

typedef enum { Hour, Minute } TimeType;

typedef enum {
  AppKeyMinuteHandColor = 0,
  AppKeyHourHandColor,
  AppKeyDateDisplayed,
  AppKeyBluetoothIcon,
  AppKeyRainbowMode,
  AppKeyBackgroundColor,
  AppKeyDateColor,
  AppKeyTimeColor,
  AppKeyInfoColor,
  AppKeyTemperatureUnit,
  AppKeyRefreshRate,
  AppKeyWeatherEnabled,
  AppKeyWeatherTemperature,
  AppKeyWeatherIcon,
  AppKeyWeatherFailed,
  AppKeyWeatherRequest,
  AppKeyJsReady
} AppKey;

typedef enum {
  PersistKeyConfig = 0,
  PersistKeyWeather
} PersistKey;

typedef struct {
  int32_t minute_hand_color;
  int32_t hour_hand_color;
  int32_t background_color;
  int32_t date_color;
  int32_t time_color;
  int32_t info_color;
  int32_t refresh_rate; // in minutes
  int8_t weather_enabled;
  int8_t temperature_unit;
  int8_t date_displayed;
  int8_t bluetooth_icon;
  int8_t rainbow_mode;
} __attribute__((__packed__)) Config;

typedef struct {
  int32_t timestamp;
  int8_t icon;
  int8_t temperature;
} __attribute__((__packed__)) Weather;

static const int HOUR_CIRCLE_RADIUS = 5;
static const int HOUR_HAND_STROKE = 6;
static const int HOUR_HAND_RADIUS = 39;
static const int MINUTE_HAND_STROKE = 6;
static const int MINUTE_HAND_RADIUS = 52;
static const int ICON_OFFSET = -18;
static const int TICK_STROKE = 2;
static const int TICK_LENGTH = 6;
static const int DATE_Y_OFFSET = 28;

static Window * s_main_window;
static Layer * s_root_layer;
static GRect s_root_layer_bounds;
static GPoint s_center;

static TextLayer * s_info_layer;

static Layer * s_tick_layer;

static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static RotBitmapLayer * s_rainbow_hand_layer;
static Layer * s_center_circle_layer;

static Layer * s_time_layer;

static Config s_config;
static Weather s_weather;
static ConfigUpdatedCallback s_config_updated_callback;

static bool s_bt_connected;

static AppTimer * s_weather_timer;

static int s_weather_failure_count;
static int s_can_send_request;
static int s_js_ready;

static void update_info_layer();

static void try_send_weather_request();
static void send_weather_request();

static void send_weather_request_callback(void * context){
  try_send_weather_request();
}

static void schedule_weather_request(int timeout){
  s_weather_timer = app_timer_register(timeout, send_weather_request_callback, NULL);
}

static int weather_expiration(){
  int timeout = s_config.refresh_rate * 60;
  d("timeout: %d", timeout);
  d("expiration: %d", (int)s_weather.timestamp + timeout);
  return s_weather.timestamp + timeout;
}

static bool weather_timedout(){
  int expiration = weather_expiration();
  return time(NULL) > expiration;
}

static void fetch_int32(const DictionaryIterator * iter, const int key, int32_t * config){
  Tuple * tuple = dict_find(iter, key);
  if(tuple){
    *config = tuple->value->int32;
  }
}

static void fetch_int8(const DictionaryIterator * iter, const int key, int8_t * config){
  Tuple * tuple = dict_find(iter, key);
  if(tuple){
    *config = tuple->value->int8;
  }
}

static void fetch_config_or_default(){
  if(persist_exists(PersistKeyWeather)){
    persist_read_data(PersistKeyWeather, &s_weather, sizeof(Weather));
  }
  if(persist_exists(PersistKeyConfig)){
    persist_read_data(PersistKeyConfig, &s_config, sizeof(Config));
  }else{
    s_config.minute_hand_color = 0xffffff;
    s_config.hour_hand_color   = 0xff0000;
    s_config.background_color  = 0x000000;
    s_config.date_color        = 0x555555;
    s_config.time_color        = 0xAAAAAA;
    s_config.info_color        = 0x555555;
    s_config.date_displayed    = true;
    s_config.bluetooth_icon    = Bluetooth;
    s_config.temperature_unit  = Celsius;
    s_config.rainbow_mode      = false;
    s_config.weather_enabled   = true;
    s_config.refresh_rate      = 20;
    persist_write_data(PersistKeyConfig, &s_config, sizeof(s_config));
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple * tuple = dict_read_first(iter);
  bool config_message = false;
  while (tuple) {
    if(tuple->key < AppKeyInfoColor){
      config_message = true;
    }
    switch (tuple->key) {
    case AppKeyJsReady:
      s_js_ready = true;
      try_send_weather_request();
      break;
    case AppKeyWeatherFailed:
      s_can_send_request = true;
      s_weather_failure_count++;
      if(s_weather_failure_count < 5){
        schedule_weather_request(1000);
      }
      break;
    case AppKeyWeatherTemperature:
      s_can_send_request = true;
      Tuple * icon_tuple = dict_find(iter, AppKeyWeatherIcon);
      Tuple * temp_tuple = dict_find(iter, AppKeyWeatherTemperature);
      if(icon_tuple && temp_tuple){
        s_weather.timestamp = time(NULL);
        s_weather.icon = icon_tuple->value->int8;
        s_weather.temperature = temp_tuple->value->int8;
      }
      persist_write_data(PersistKeyWeather, &s_weather, sizeof(s_weather));
      update_info_layer();
      break;
    case AppKeyInfoColor:
      s_config.info_color = tuple->value->int32;
      update_info_layer();
    case AppKeyRefreshRate:
      s_config.refresh_rate = tuple->value->int32;
    case AppKeyWeatherEnabled:
      s_config.weather_enabled = tuple->value->int8;
      update_info_layer();
    }
    tuple = dict_read_next(iter);
  }
  fetch_int32(iter, AppKeyMinuteHandColor, &s_config.minute_hand_color);
  fetch_int32(iter, AppKeyHourHandColor, &s_config.hour_hand_color);
  fetch_int32(iter, AppKeyBackgroundColor, &s_config.background_color);
  fetch_int32(iter, AppKeyDateColor, &s_config.date_color);
  fetch_int32(iter, AppKeyTimeColor, &s_config.time_color);
  fetch_int8(iter, AppKeyTemperatureUnit, &s_config.temperature_unit);
  fetch_int8(iter, AppKeyDateDisplayed, &s_config.date_displayed);
  fetch_int8(iter, AppKeyBluetoothIcon, &s_config.bluetooth_icon);
  fetch_int8(iter, AppKeyRainbowMode, &s_config.rainbow_mode);
  persist_write_data(PersistKeyConfig, &s_config, sizeof(s_config));
  s_config_updated_callback();
  if(config_message){
    s_weather.timestamp = time(NULL);
    send_weather_request();
  }
}

static GColor config_get_minute_hand_color(){
  return GColorFromHEX(s_config.minute_hand_color);
}

static GColor config_get_hour_hand_color(){
  return GColorFromHEX(s_config.hour_hand_color);
}

static GColor config_get_background_color(){
  return GColorFromHEX(s_config.background_color);
}

static GColor config_get_date_color(){
  return GColorFromHEX(s_config.date_color);
}

static GColor config_get_time_color(){
  return GColorFromHEX(s_config.time_color);
}

static bool config_is_date_displayed(){
  return s_config.date_displayed;
}

static BluetoothIcon config_get_bluetooth_icon(){
  return s_config.bluetooth_icon;
}

static bool config_is_rainbow_mode(){
  return s_config.rainbow_mode;
}

static void init_config(ConfigUpdatedCallback callback){
  s_config_updated_callback = callback;
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  fetch_config_or_default();
}

// Hands
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
  return graphics_text_layout_get_content_size(text, get_font(), s_root_layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
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

static void init_times(){
  s_time_layer = layer_create(s_root_layer_bounds);
  layer_set_update_proc(s_time_layer, time_layer_update_callback);
  layer_add_child(s_root_layer, s_time_layer);
}

static void deinit_times(){
  layer_destroy(s_time_layer);
}

static void mark_dirty_time_layer(){
  if(s_time_layer){
    layer_mark_dirty(s_time_layer);
  }
}

static void hands_update_time_changed(){
 if(s_hour_hand_layer){
   layer_mark_dirty(s_hour_hand_layer);
 }
 if(s_minute_hand_layer){
   layer_mark_dirty(s_minute_hand_layer);
 }
 if(s_rainbow_hand_layer){
   const Time current_time = get_current_time();
   const float hand_angle = angle(current_time.minute, 60);
   const bool rainbow_mode = config_is_rainbow_mode();
   rot_bitmap_layer_set_angle(s_rainbow_hand_layer, hand_angle);
   layer_set_hidden((Layer*)s_rainbow_hand_layer, !rainbow_mode);
 }
}

static void hands_update_minute_hand_config_changed(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
  }
}

static void hands_update_hour_hand_config_changed(){
  if(s_minute_hand_layer){
    layer_mark_dirty(s_minute_hand_layer);
  }
}

static void hands_update_rainbow_mode_config_changed(){
  const Time current_time = get_current_time();
  const float hand_angle = angle(current_time.minute, 60);
  const bool rainbow_mode = config_is_rainbow_mode();
  if(s_minute_hand_layer){
    layer_set_hidden(s_minute_hand_layer, rainbow_mode);
    layer_mark_dirty(s_minute_hand_layer);
  }
  if(s_rainbow_hand_layer){
    rot_bitmap_layer_set_angle(s_rainbow_hand_layer, hand_angle);
    layer_set_hidden((Layer*)s_rainbow_hand_layer, !rainbow_mode);
  }
  if(s_center_circle_layer){
    layer_mark_dirty(s_center_circle_layer);
  }
}


static void update_minute_hand_layer(Layer *layer, GContext * ctx){
  const Time current_time = get_current_time();
  const float hand_angle = angle(current_time.minute, 60);
  const GPoint hand_end = gpoint_on_circle(s_center, hand_angle, MINUTE_HAND_RADIUS);
  set_stroke_width(ctx, MINUTE_HAND_STROKE);
  set_stroke_color(ctx, config_get_minute_hand_color());
  draw_line(ctx, s_center, hand_end);
}

static void update_hour_hand_layer(Layer * layer, GContext * ctx){
  const Time current_time = get_current_time();
  const float hand_angle = angle(current_time.hour * 50 + current_time.minute * 50 / 60, 600);
  const GPoint hand_end = gpoint_on_circle(s_center, hand_angle, HOUR_HAND_RADIUS);
  set_stroke_width(ctx, HOUR_HAND_STROKE);
  set_stroke_color(ctx, config_get_hour_hand_color());
  draw_line(ctx, s_center, hand_end);
}

static void update_center_circle_layer(Layer * layer, GContext * ctx){
  if(config_is_rainbow_mode()){
    graphics_context_set_fill_color(ctx, GColorVividViolet);
  }else{
    graphics_context_set_fill_color(ctx, config_get_hour_hand_color());
  }
  graphics_fill_circle(ctx, s_center, HOUR_CIRCLE_RADIUS);
}

static void init_hands(){
  s_rainbow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_RAINBOW_HAND);

  s_minute_hand_layer   = layer_create(s_root_layer_bounds);
  s_hour_hand_layer     = layer_create(s_root_layer_bounds);
  s_center_circle_layer = layer_create(s_root_layer_bounds);
  s_rainbow_hand_layer  = rot_bitmap_layer_create(s_rainbow_bitmap);
  rot_bitmap_set_compositing_mode(s_rainbow_hand_layer, GCompOpSet);
  rot_bitmap_set_src_ic(s_rainbow_hand_layer, GPoint(5, 55));
  GRect frame = layer_get_frame((Layer *) s_rainbow_hand_layer);
  frame.origin.x = s_center.x - frame.size.w / 2;
  frame.origin.y = s_center.y - frame.size.h / 2;
  layer_set_frame((Layer *)s_rainbow_hand_layer, frame);

  layer_set_update_proc(s_hour_hand_layer,     update_hour_hand_layer);
  layer_set_update_proc(s_minute_hand_layer,   update_minute_hand_layer);
  layer_set_update_proc(s_center_circle_layer, update_center_circle_layer);

  layer_add_child(s_root_layer, s_minute_hand_layer);
  layer_add_child(s_root_layer, (Layer *)s_rainbow_hand_layer);
  layer_add_child(s_root_layer, s_hour_hand_layer);
  layer_add_child(s_root_layer, s_center_circle_layer);

  hands_update_rainbow_mode_config_changed();
}

static void deinit_hands(){
  layer_destroy(s_hour_hand_layer);
  rot_bitmap_layer_destroy(s_rainbow_hand_layer);
  layer_destroy(s_minute_hand_layer);
  layer_destroy(s_center_circle_layer);
  gbitmap_destroy(s_rainbow_bitmap);
}

// Ticks
static void mark_dirty_tick_layer();

static void draw_tick(GContext *ctx, const int index){
  draw_line(ctx, ticks_points[index][0], ticks_points[index][1]);
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  const Time current_time = get_current_time();
  set_stroke_color(ctx, config_get_time_color());
  set_stroke_width(ctx, TICK_STROKE);
  const int hour_tick_index = current_time.hour % 12;
  draw_tick(ctx, hour_tick_index);
  const int minute_tick_index = current_time.minute / 5;
  if(hour_tick_index != minute_tick_index){
    draw_tick(ctx, minute_tick_index);
  }
}

static void init_tick_layer(){
  s_tick_layer = layer_create(s_root_layer_bounds);
  layer_set_update_proc(s_tick_layer, tick_layer_update_callback);
  layer_add_child(s_root_layer, s_tick_layer);
}

static void deinit_tick_layer(){
  layer_destroy(s_tick_layer);
}

static void mark_dirty_tick_layer(){
  if(s_tick_layer){
    layer_mark_dirty(s_tick_layer);
  }
}

// Infos: bluetooth + weather

static void update_info_layer(){
  text_layer_set_text_color(s_info_layer, GColorFromHEX(s_config.info_color));
  static char s_info_buffer[10];
  int idx = 0;
  s_info_buffer[0] = 0;

  const BluetoothIcon new_icon = config_get_bluetooth_icon();
  if(!s_bt_connected && new_icon == Bluetooth){
    s_info_buffer[idx++] = 'z';
  }else if(!s_bt_connected && new_icon == Heart){
    s_info_buffer[idx++] = 'Z';
  }
  if(!weather_timedout() && s_config.weather_enabled){
    s_info_buffer[idx++] = s_weather.icon;

    // itoa
    int temp = s_weather.temperature;
    if(s_config.temperature_unit == Fahrenheit){
      temp = tempToF(temp);
    }
    if(temp < 0){
      s_info_buffer[idx++] = '-';
      temp = -temp;
    }else if(temp == 0){
      s_info_buffer[idx++] = '0';
    }
    char temp_buffer[5];
    int idx_temp = 0;
    while(temp != 0 && idx_temp < 5){
      char next_digit = (char)(temp % 10);
      temp_buffer[idx_temp++] = next_digit + '0';
      temp /= 10;
    }
    while(idx_temp > 0){
      s_info_buffer[idx++] = temp_buffer[--idx_temp];
    }
    s_info_buffer[idx] = 0;
    strcat(s_info_buffer, "°");
  }else{
    s_info_buffer[idx] = 0;
  }
  text_layer_set_text(s_info_layer, s_info_buffer);
}

static void bt_handler(bool connected){
  s_bt_connected = connected;
  update_info_layer();
}

static bool should_not_update_weather(){
  const bool almost_expired = time(NULL) > weather_expiration() - 60;
  return !almost_expired || !(s_can_send_request && s_js_ready);
}

static void send_weather_request(){
  if(s_config.weather_enabled){
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if(result == APP_MSG_OK) {
      s_can_send_request = false;
      const int value = 1;
      dict_write_int(out_iter, AppKeyWeatherRequest, &value, sizeof(int), true);
      result = app_message_outbox_send();
      if(result != APP_MSG_OK) {
        s_can_send_request = true;
        schedule_weather_request(100);
        e("Error sending the outbox: %d", (int)result);
      }
    } else {
      schedule_weather_request(100);
      e("Error preparing the outbox: %d", (int)result);
    }
  }
}

static void try_send_weather_request(){
  if(should_not_update_weather()){
    return;
  }
  send_weather_request();
}

static void init_info_layer(){
  const GSize size = GSize(100, 23);
  const GRect rect_at_center = (GRect) { .origin = s_center, .size = size };
  const GRect bounds = grect_translated(rect_at_center, - size.w / 2, - size.h + ICON_OFFSET);

  s_info_layer = text_layer_create(bounds);
  text_layer_set_text_alignment(s_info_layer, GTextAlignmentCenter);
  text_layer_set_font(s_info_layer, get_font());
  text_layer_set_overflow_mode(s_info_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_info_layer, GColorClear);
  layer_add_child(s_root_layer, text_layer_get_layer(s_info_layer));

  bluetooth_connection_service_subscribe(bt_handler);
  bt_handler(connection_service_peek_pebble_app_connection());
}

static void deinit_info_layer(){
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_info_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  schedule_weather_request(100);
  update_current_time();
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_time_changed();
  update_info_layer();
}

static void config_updated_callback(){
  layer_mark_dirty(s_root_layer);
  mark_dirty_time_layer();
  mark_dirty_tick_layer();
  hands_update_rainbow_mode_config_changed();
  hands_update_minute_hand_config_changed();
  hands_update_hour_hand_config_changed();
  window_set_background_color(s_main_window, config_get_background_color());
}

static void main_window_load(Window *window) {
  s_root_layer = window_get_root_layer(window);
  s_root_layer_bounds = layer_get_bounds(s_root_layer);
  s_center = grect_center_point(&s_root_layer_bounds);
  update_current_time();
  window_set_background_color(window, config_get_background_color());

  init_font();
  init_info_layer();
  init_times();
  init_tick_layer();
  init_hands();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_current_time();
}

static void main_window_unload(Window *window) {
  deinit_hands();
  deinit_times();
  deinit_tick_layer();
  deinit_info_layer();
  deinit_font();
}

static void init() {
  s_weather_failure_count = 0;
  s_js_ready = false;
  s_can_send_request = true;
  init_config(config_updated_callback);
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
