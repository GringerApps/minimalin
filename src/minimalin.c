#include <pebble.h>
#include "common.h"
#include "config.h"

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
static GPoint SOUTH_INFO_CENTER = { .x = 90, .y = 118 };
static GPoint NORTH_INFO_CENTER = { .x = 90, .y = 62 };
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
static GPoint SOUTH_INFO_CENTER = { .x = 72, .y = 112 };
static GPoint NORTH_INFO_CENTER = { .x = 72, .y = 56 };
#endif

typedef struct {
  TextLayer * layer;
  char text[20];
} TextBlock;

static TextBlock * text_block_create(Layer * parent_layer, const GPoint center, const GFont font){
  TextBlock * text_block = (TextBlock *) malloc(sizeof(TextBlock));
  const GSize size = GSize(100, 23);
  const GRect bounds = (GRect) {
    .origin = GPoint(center.x - size.w / 2 , center.y - size.h / 2),
    .size   = size
  };
  text_block->layer = text_layer_create(bounds);
  text_layer_set_font(text_block->layer, font);
  text_layer_set_text_alignment(text_block->layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_block->layer, GColorClear);
  layer_add_child(parent_layer, text_layer_get_layer(text_block->layer));
  return text_block;
}

static TextBlock * text_block_destroy(TextBlock * text_block){
  text_layer_destroy(text_block->layer);
  free(text_block);
  return NULL;
}

static void text_block_set_text(TextBlock * text_block, const char * text, const GColor color){
  strncpy(text_block->text, text, sizeof(text_block->text));
  text_layer_set_text_color(text_block->layer, color);
  text_layer_set_text(text_block->layer, text_block->text);
}

static void text_block_set_visible(TextBlock * text_block, const bool visible){
  layer_set_hidden(text_layer_get_layer(text_block->layer), !visible);
}

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

static Window * s_main_window;
static Layer * s_root_layer;
static GRect s_root_layer_bounds;
static GPoint s_center;

static TextBlock * s_north_info;
static TextBlock * s_south_info;

static Layer * s_tick_layer;

static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static RotBitmapLayer * s_rainbow_hand_layer;
static Layer * s_center_circle_layer;

static Layer * s_time_layer;

static Config * s_config;
static Weather s_weather;

static bool s_bt_connected;

static AppTimer * s_weather_timer;

static int s_weather_failure_count;
static int s_can_send_request;
static int s_js_ready;

static GFont s_font;

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
  int timeout = config_get_int(s_config, ConfigIntKeyRefreshRate) * 60;
  return s_weather.timestamp + timeout;
}

static bool weather_timedout(){
  int expiration = weather_expiration();
  return time(NULL) > expiration;
}

static void config_updated_callback();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple * tuple = dict_read_first(iter);
  bool config_message = false;
  while (tuple) {
    if(tuple->key < AppKeyWeatherTemperature){
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
      s_config->info_color = tuple->value->int32;
      break;
    case AppKeyRefreshRate:
      config_set_int(s_config, ConfigIntKeyRefreshRate, tuple->value->int32);
      break;
    case AppKeyTemperatureUnit:
      config_set_int(s_config, ConfigIntKeyTemperatureUnit, tuple->value->int32);
      break;
    case AppKeyBluetoothIcon:
      config_set_int(s_config, ConfigIntKeyBluetoothIcon, tuple->value->int32);
      break;
    case AppKeyDateDisplayed:
      config_set_bool(s_config, ConfigBoolKeyDateDisplayed, tuple->value->int8);
      text_block_set_visible(s_south_info, tuple->value->int8);
      break;
    case AppKeyRainbowMode:
      config_set_bool(s_config, ConfigBoolKeyRainbowMode, tuple->value->int8);
      break;
    case AppKeyBackgroundColor:
      config_set_color(s_config, ConfigColorKeyBackground, tuple->value->int32);
      break;
    case AppKeyTimeColor:
      config_set_color(s_config, ConfigColorKeyTime, tuple->value->int32);
      break;
    case AppKeyDateColor:
      config_set_color(s_config, ConfigColorKeyDate, tuple->value->int32);
      break;
    case AppKeyHourHandColor:
      config_set_color(s_config, ConfigColorKeyHourHand, tuple->value->int32);
      break;
    case AppKeyMinuteHandColor:
      config_set_color(s_config, ConfigColorKeyMinuteHand, tuple->value->int32);
      break;
    case AppKeyWeatherEnabled:
      config_set_bool(s_config, ConfigBoolKeyWeatherEnabled, tuple->value->int8);
    }
    tuple = dict_read_next(iter);
  }
  if(config_message){
    config_save(s_config, PersistKeyConfig);
    config_updated_callback();
    send_weather_request();
  }
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
  return graphics_text_layout_get_content_size(text, s_font, s_root_layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
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
  draw_text(ctx, buffer, s_font, box);
}

static void display_time(GContext * ctx, const int hour, const int minute){
  graphics_context_set_text_color(ctx, config_get_color(s_config, ConfigColorKeyTime));
  char buffer[] = "00:00";
  if(time_displayed_horizontally(hour, minute)){
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    const GPoint box_center = get_time_point(hour, Hour);
    const GRect box         = get_display_box(box_center, "00:00");
    draw_text(ctx, buffer, s_font, box);
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
  const GColor date_color = config_get_color(s_config, ConfigColorKeyDate);
  char buffer[] = "00";
  snprintf(buffer, sizeof(buffer), "%d", day);
  text_block_set_text(s_south_info, buffer, date_color);
}

static void time_layer_update_callback(Layer * layer, GContext *ctx){
  Time current_time = get_current_time();
  display_time(ctx, current_time.hour, current_time.minute);
  if(config_get_bool(s_config, ConfigBoolKeyDateDisplayed)){
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
   const bool rainbow_mode = config_get_bool(s_config, ConfigBoolKeyRainbowMode);
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
  const bool rainbow_mode = config_get_bool(s_config, ConfigBoolKeyRainbowMode);
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
  set_stroke_color(ctx, config_get_color(s_config, ConfigColorKeyMinuteHand));
  draw_line(ctx, s_center, hand_end);
}

static void update_hour_hand_layer(Layer * layer, GContext * ctx){
  const Time current_time = get_current_time();
  const float hand_angle = angle(current_time.hour * 50 + current_time.minute * 50 / 60, 600);
  const GPoint hand_end = gpoint_on_circle(s_center, hand_angle, HOUR_HAND_RADIUS);
  set_stroke_width(ctx, HOUR_HAND_STROKE);
  set_stroke_color(ctx, config_get_color(s_config, ConfigColorKeyHourHand));
  draw_line(ctx, s_center, hand_end);
}

static void update_center_circle_layer(Layer * layer, GContext * ctx){
  if(config_get_bool(s_config, ConfigBoolKeyRainbowMode)){
    graphics_context_set_fill_color(ctx, GColorVividViolet);
  }else{
    graphics_context_set_fill_color(ctx, config_get_color(s_config, ConfigColorKeyHourHand));
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
  set_stroke_color(ctx, config_get_color(s_config, ConfigColorKeyTime));
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
  char s_info_buffer[10] = {0};
  int idx = 0;

  const BluetoothIcon new_icon = config_get_int(s_config, ConfigIntKeyBluetoothIcon);
  if(!s_bt_connected && new_icon == Bluetooth){
    s_info_buffer[idx++] = 'z';
  }else if(!s_bt_connected && new_icon == Heart){
    s_info_buffer[idx++] = 'Z';
  }
  if(!weather_timedout() && config_get_bool(s_config, ConfigBoolKeyWeatherEnabled)){
    s_info_buffer[idx++] = s_weather.icon;

    // itoa
    int temp = s_weather.temperature;
    if(config_get_int(s_config, ConfigIntKeyTemperatureUnit) == Fahrenheit){
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
  const GColor info_color = config_get_color(s_config, ConfigColorKeyInfo);
  text_block_set_text(s_north_info, s_info_buffer, info_color);
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
  if(config_get_bool(s_config, ConfigBoolKeyWeatherEnabled)){
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
  update_info_layer();
  hands_update_rainbow_mode_config_changed();
  hands_update_minute_hand_config_changed();
  hands_update_hour_hand_config_changed();
  window_set_background_color(s_main_window, config_get_color(s_config, ConfigColorKeyBackground));
}

static void main_window_load(Window *window) {
  s_root_layer = window_get_root_layer(window);
  s_root_layer_bounds = layer_get_bounds(s_root_layer);
  s_center = grect_center_point(&s_root_layer_bounds);
  update_current_time();
  window_set_background_color(window, config_get_color(s_config, ConfigColorKeyBackground));

  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NUPE_23));

  s_south_info = text_block_create(s_root_layer, SOUTH_INFO_CENTER, s_font);
  text_block_set_visible(s_south_info, config_get_bool(s_config, ConfigBoolKeyDateDisplayed));

  s_north_info = text_block_create(s_root_layer, NORTH_INFO_CENTER, s_font);
  bluetooth_connection_service_subscribe(bt_handler);
  bt_handler(connection_service_peek_pebble_app_connection());

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

  bluetooth_connection_service_unsubscribe();
  text_block_destroy(s_south_info);
  text_block_destroy(s_north_info);

  fonts_unload_custom_font(s_font);
}

static void init() {
  s_weather_failure_count = 0;
  s_js_ready = false;
  s_can_send_request = true;
  s_config = config_load(PersistKeyConfig);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  config_destroy(s_config);
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
