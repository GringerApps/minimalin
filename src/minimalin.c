#include <pebble.h>
#include "common.h"
#include "config.h"
#include "text_block.h"

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
static TextBlock * s_hour_text;
static TextBlock * s_minute_text;

static Layer * s_tick_layer;

static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static RotBitmapLayer * s_rainbow_hand_layer;
static Layer * s_center_circle_layer;

static Config * s_config;
static Weather s_weather;

static bool s_bt_connected;

static AppTimer * s_weather_request_timer;
static int s_weather_request_timeout;

static int s_js_ready;

static GFont s_font;

static void update_info_layer();

static void schedule_weather_request(int timeout);

static void send_weather_request_callback(void * context){
  s_weather_request_timer = NULL;
  const int timeout = config_get_int(s_config, ConfigIntKeyRefreshRate) * 60;
  const int expiration =  s_weather.timestamp + timeout;
  const bool almost_expired = time(NULL) > expiration;
  const bool can_update_weather = almost_expired && s_js_ready;
  if(can_update_weather){
    if(config_get_bool(s_config, ConfigBoolKeyWeatherEnabled)){
      DictionaryIterator *out_iter;
      AppMessageResult result = app_message_outbox_begin(&out_iter);
      if(result == APP_MSG_OK) {
        const int value = 1;
        dict_write_int(out_iter, AppKeyWeatherRequest, &value, sizeof(int), true);
        result = app_message_outbox_send();
        if(result != APP_MSG_OK) {
          schedule_weather_request(1000);
          e("Error sending the outbox: %d", (int)result);
        }
      } else {
        schedule_weather_request(1000);
        e("Error preparing the outbox: %d", (int)result);
      }
    }
  }
}

static void schedule_weather_request(int timeout){
  if(s_weather_request_timer){
    int expiration = time(NULL) + timeout;
    if(expiration < s_weather_request_timeout){
      s_weather_request_timeout = expiration;
      app_timer_reschedule(s_weather_request_timer, timeout);
    }
  }else{
    s_weather_request_timer = app_timer_register(timeout, send_weather_request_callback, NULL);
  }
}
static void config_updated_callback();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple * tuple = dict_read_first(iter);
  bool config_message = false;
  Tuple * icon_tuple;
  Tuple * temp_tuple;
  while (tuple) {
    if(tuple->key < AppKeyWeatherTemperature){
      config_message = true;
    }
    switch (tuple->key) {
    case AppKeyJsReady:
      s_js_ready = true;
      schedule_weather_request(0);
      break;
    case AppKeyWeatherFailed:
      schedule_weather_request(60000);
      break;
    case AppKeyWeatherTemperature:
      icon_tuple = dict_find(iter, AppKeyWeatherIcon);
      temp_tuple = dict_find(iter, AppKeyWeatherTemperature);
      if(icon_tuple && temp_tuple){
        s_weather.timestamp = time(NULL);
        s_weather.icon = icon_tuple->value->int8;
        s_weather.temperature = temp_tuple->value->int8;
      }
      persist_write_data(PersistKeyWeather, &s_weather, sizeof(Weather));
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
    schedule_weather_request(0);
  }
}

// Hands
static void update_times(){
  Time current_time = get_current_time();
  int hour = current_time.hour;
  int minute = current_time.minute;
  GColor color = config_get_color(s_config, ConfigColorKeyTime);
  char buffer[] = "00:00";
  GPoint hour_box_center   = time_points[hour % 12];
  GPoint minute_box_center = time_points[minute / 5];
  const bool time_conflicts = hour % 12 == minute / 5;
  const bool horizontal_display = time_conflicts && (hour <= 1 || hour >= 11 || (hour >= 5 && hour <= 7));
  if(horizontal_display){
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    text_block_set_text(s_hour_text, buffer, color);
    text_block_set_visible(s_minute_text, false);
  }else{
    text_block_set_visible(s_minute_text, true);
    bool vertical_display = time_conflicts && ((hour > 1 && hour < 5) || (hour > 7 && hour < 11));
    if(vertical_display){
      hour_box_center.y -= 12;
      minute_box_center.y += 12;
    }
    snprintf(buffer, 3, "%d", current_time.hour);
    text_block_set_text(s_hour_text, buffer, color);
    snprintf(buffer, 3, "%02d", current_time.minute);
    text_block_set_text(s_minute_text, buffer, color);
  }
  text_block_move(s_hour_text, hour_box_center);
  text_block_move(s_minute_text, minute_box_center);
  if(config_get_bool(s_config, ConfigBoolKeyDateDisplayed)){
    const GColor date_color = config_get_color(s_config, ConfigColorKeyDate);
    char buffer[] = "00";
    snprintf(buffer, sizeof(buffer), "%d", current_time.day);
    text_block_set_text(s_south_info, buffer, date_color);
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
  char info_buffer[10] = {0};
  const BluetoothIcon new_icon = config_get_int(s_config, ConfigIntKeyBluetoothIcon);
  if(!s_bt_connected){
    if(new_icon == Bluetooth){
      strncat(info_buffer, "z", 2);
    }else if(new_icon == Heart){
      strncat(info_buffer, "Z", 2);
    }
  }
  const int timeout = (config_get_int(s_config, ConfigIntKeyRefreshRate) + 5) * 60;
  const int expiration =  s_weather.timestamp + timeout;
  const bool weather_valid = time(NULL) < expiration;
  if(weather_valid && config_get_bool(s_config, ConfigBoolKeyWeatherEnabled)){
    int temp = s_weather.temperature;
    if(config_get_int(s_config, ConfigIntKeyTemperatureUnit) == Fahrenheit){
      temp = tempToF(temp);
    }
    char temp_buffer[6];
    snprintf(temp_buffer, 6, "%c%d°", s_weather.icon, temp);
    strcat(info_buffer, temp_buffer);
  }
  const GColor info_color = config_get_color(s_config, ConfigColorKeyInfo);
  text_block_set_text(s_north_info, info_buffer, info_color);
}

static void bt_handler(bool connected){
  s_bt_connected = connected;
  update_info_layer();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  schedule_weather_request(100);
  update_current_time();
  update_times();
  mark_dirty_tick_layer();
  hands_update_time_changed();
  update_info_layer();
}

static void config_updated_callback(){
  layer_mark_dirty(s_root_layer);
  update_times();
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

  s_hour_text = text_block_create(s_root_layer, time_points[6] , s_font);
  s_minute_text = text_block_create(s_root_layer, time_points[0] , s_font);

  init_tick_layer();
  init_hands();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_current_time();
  update_times();
}

static void main_window_unload(Window *window) {
  deinit_hands();

  text_block_destroy(s_hour_text);
  text_block_destroy(s_minute_text);

  deinit_tick_layer();

  bluetooth_connection_service_unsubscribe();
  text_block_destroy(s_south_info);
  text_block_destroy(s_north_info);

  fonts_unload_custom_font(s_font);
}

static void init() {
  s_weather_request_timeout = 0;
  s_js_ready = false;
  s_config = config_load(PersistKeyConfig);
  if(persist_exists(PersistKeyWeather)){
    persist_read_data(PersistKeyWeather, &s_weather, sizeof(Weather));
  }
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
