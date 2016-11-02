#include <pebble.h>
#include "config.h"
#include "text_block.h"
#include "quadrant.h"
#include "messenger.h"
#include "minimalin.h"
#include "geometry.h"

// #define d(string, ...) APP_LOG (APP_LOG_LEVEL_DEBUG, string, ##__VA_ARGS__)
// #define e(string, ...) APP_LOG (APP_LOG_LEVEL_ERROR, string, ##__VA_ARGS__)
// #define i(string, ...) APP_LOG (APP_LOG_LEVEL_INFO, string, ##__VA_ARGS__)

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
  {90,  17} ,
  {124, 28} ,
  {150, 50} ,
  {161, 86} ,
  {148, 124},
  {124, 146},
  {90,  159},
  {54,  147},
  {29,  124},
  {18,  87} ,
  {30,  52} ,
  {54,  28} ,
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

typedef enum {
  AppKeyMinuteHandColor = 0,
  AppKeyHourHandColor,
  AppKeyDateDisplayed,
  AppKeyBluetoothIcon,
  AppKeyRainbowMode,
  AppKeyBackgroundColor,
  AppKeyTimeColor,
  AppKeyInfoColor,
  AppKeyTemperatureUnit,
  AppKeyRefreshRate,
  AppKeyWeatherEnabled,
  AppKeyConfig,
  AppKeyWeatherTemperature,
  AppKeyWeatherIcon,
  AppKeyWeatherFailed,
  AppKeyWeatherRequest,
  AppKeyJsReady,
  AppKeyVibrateOnTheHour,
  AppKeyMilitaryTime,
  AppKeyHealthEnabled
} AppKey;

typedef enum {
  PersistKeyConfig = 0,
  PersistKeyWeather
} PersistKey;

typedef struct {
  int hour;
  int minute;
  int day;
} Time;

typedef struct {
  int32_t timestamp;
  int8_t icon;
  int8_t temperature;
} __attribute__((__packed__)) Weather;

static Window * s_main_window;
static Layer * s_root_layer;
static GRect s_root_layer_bounds;
static GPoint s_center;

static TextBlock * s_weather_info;
static TextBlock * s_date_info;
static TextBlock * s_steps_info;
static TextBlock * s_watch_info;
static TextBlock * s_hour_text;
static TextBlock * s_minute_text;

static Layer * s_tick_layer;

static GBitmap * s_rainbow_bitmap;
static Layer * s_minute_hand_layer;
static Layer * s_hour_hand_layer;
static RotBitmapLayer * s_rainbow_hand_layer;
static Layer * s_center_circle_layer;

static Quadrants * s_quadrants;

static Config * s_config;
static Messenger * s_messenger;
static Weather s_weather;

static bool s_bt_connected;
static int s_battery_percent;

static AppTimer * s_weather_request_timer;
static int s_weather_request_timeout;

static int s_js_ready;

static GFont s_font;

static tm * s_current_time;

static void update_weather_layer();
static void schedule_weather_request(int timeout);
static void update_times();
static void update_date();
static void mark_dirty_minute_hand_layer();

static void update_current_time() {
#ifdef SCREENSHOT
  time_t screenshot_time = 1454278942;
  s_current_time = gmtime(&screenshot_time);
#else
  const time_t temp = time(NULL);
  s_current_time = localtime(&temp);
#endif
}

static int index_hour(){
  return s_current_time->tm_hour % 12;
}

static int index_minute(){
  return s_current_time->tm_min / 5;
}

static bool time_conflicts(){
  return index_hour() == index_minute();
}

typedef enum { Min = 0, Hour } TmMember;
typedef struct {
  int lower_bound;
  int upper_bound;
} Range;
typedef enum { IndexWeather, IndexSteps, IndexDate } BlockIndex;

static void send_weather_request_callback(void * context){
  s_weather_request_timer = NULL;
  const int timeout = config_get_int(s_config, ConfigKeyRefreshRate) * 60;
  const int expiration =  s_weather.timestamp + timeout;
  const bool almost_expired = time(NULL) > expiration;
  const bool can_update_weather = almost_expired && s_js_ready;
  if(can_update_weather){
    if(config_get_bool(s_config, ConfigKeyWeatherEnabled)){
      DictionaryIterator *out_iter;
      AppMessageResult result = app_message_outbox_begin(&out_iter);
      if(result == APP_MSG_OK) {
        const int value = 1;
        dict_write_int(out_iter, AppKeyWeatherRequest, &value, sizeof(int), true);
        result = app_message_outbox_send();
        if(result != APP_MSG_OK) {
          schedule_weather_request(5000);
          // e("Error sending the outbox: %d", (int)result);
        }
      } else {
        schedule_weather_request(5000);
        // e("Error preparing the outbox: %d", (int)result);
      }
    }
  }
}

static void schedule_weather_request(int timeout){
  int expiration = time(NULL) + timeout;
  if(s_weather_request_timer){
    if(expiration < s_weather_request_timeout){
      s_weather_request_timeout = expiration;
      app_timer_reschedule(s_weather_request_timer, timeout);
    }
  }else{
    s_weather_request_timeout = expiration;
    s_weather_request_timer = app_timer_register(timeout, send_weather_request_callback, NULL);
  }
}

static void step_handler(HealthEventType event, void *context);
static void update_weather_layer();
static void update_watch_layer();
static void update_date();

static void config_info_color_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyInfoColor, tuple->value->int32);
  update_date();
  step_handler(HealthEventSignificantUpdate, NULL);
  update_weather_layer();
  update_watch_layer();
}

static void config_background_color_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyBackgroundColor, tuple->value->int32);
  window_set_background_color(s_main_window, config_get_color(s_config, ConfigKeyBackgroundColor));
}

static void config_time_color_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyTimeColor, tuple->value->int32);
  layer_mark_dirty(s_tick_layer);
  update_times();
}

static void config_hour_hand_color_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyHourHandColor, tuple->value->int32);
  layer_mark_dirty(s_hour_hand_layer);
}

static void config_minute_hand_color_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyMinuteHandColor, tuple->value->int32);
  mark_dirty_minute_hand_layer();
}

static void config_refresh_rate_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyRefreshRate, tuple->value->int32);
}

static void config_temperature_unit_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyTemperatureUnit, tuple->value->int32);
  update_weather_layer();
}

static void config_bluetooth_icon_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_int(s_config, ConfigKeyBluetoothIcon, tuple->value->int32);
  text_block_set_enabled(s_watch_info, tuple->value->int8 != 0);
  update_watch_layer();
}

static void config_date_displayed_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyDateDisplayed, tuple->value->int8);
  text_block_set_enabled(s_date_info, tuple->value->int8);
  update_date();
}

static void config_rainbow_mode_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyRainbowMode, tuple->value->int8);
  mark_dirty_minute_hand_layer();
}

static void config_weather_enabled_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyWeatherEnabled, tuple->value->int8);
  text_block_set_enabled(s_weather_info, tuple->value->int8); 
  update_weather_layer();
}

static void config_hourly_vibrate_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyVibrateOnTheHour, tuple->value->int8);
}

static void config_military_time_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyMilitaryTime, tuple->value->int8);
  layer_mark_dirty(s_tick_layer);
  update_times();
}

static void config_health_enabled_updated(DictionaryIterator * iter, Tuple * tuple){
  config_set_bool(s_config, ConfigKeyHealthEnabled, tuple->value->int8);
  text_block_set_enabled(s_steps_info, tuple->value->int8);
}

static void js_ready_callback(DictionaryIterator * iter, Tuple * tuple){
  s_js_ready = true;
  schedule_weather_request(0);
}

static void weather_requested_callback(DictionaryIterator * iter, Tuple * tuple){
  Tuple * icon_tuple = dict_find(iter, AppKeyWeatherIcon);
  Tuple * temp_tuple = dict_find(iter, AppKeyWeatherTemperature);
  if(icon_tuple && temp_tuple){
    s_weather.timestamp = time(NULL);
    s_weather.icon = icon_tuple->value->int8;
    s_weather.temperature = temp_tuple->value->int8;
  }
  persist_write_data(PersistKeyWeather, &s_weather, sizeof(Weather));
  update_weather_layer();
  quadrants_update(s_quadrants, s_current_time);
}

static void messenger_callback(DictionaryIterator * iter){
  if(dict_find(iter, AppKeyConfig)){
    config_save(s_config, PersistKeyConfig);
    s_weather.timestamp = 0;
    schedule_weather_request(0);
  }
  quadrants_update(s_quadrants, s_current_time);
  layer_mark_dirty(s_root_layer);
}

// Hands
static void update_times(){
  GColor color = config_get_color(s_config, ConfigKeyTimeColor);
  char buffer[] = "00:00";
  GPoint hour_box_center   = time_points[index_hour()];
  GPoint minute_box_center = time_points[index_minute()];
  const int hour_12 = index_hour();
  const bool conflicts = time_conflicts();
  const bool horizontal_display = conflicts && (hour_12 <= 1 || hour_12 >= 11 || (hour_12 >= 5 && hour_12 <= 7));
  if(horizontal_display){
    clock_copy_time_string(buffer, sizeof(buffer));
    text_block_set_text(s_hour_text, buffer, color);
    text_block_set_enabled(s_minute_text, false);
  }else{
    text_block_set_enabled(s_minute_text, true);
    bool vertical_display = conflicts && ((hour_12 > 1 && hour_12 < 5) || (hour_12 > 7 && hour_12 < 11));
    if(vertical_display){
      hour_box_center.y -= 10;
      minute_box_center.y += 10;
    }
    int h = s_current_time->tm_hour;
    if(!config_get_bool(s_config, ConfigKeyMilitaryTime)){
      if (h > 12) {
        h -= 12;
      }else if(h == 0){
        h = 12;
      }
    }
    snprintf(buffer, sizeof(buffer), "%d", h);
    text_block_set_text(s_hour_text, buffer, color);
    snprintf(buffer, sizeof(buffer), "%02d", s_current_time->tm_min);
    text_block_set_text(s_minute_text, buffer, color);
  }
  text_block_move(s_hour_text, hour_box_center);
  text_block_move(s_minute_text, minute_box_center);
}

static void update_date(){
  if(config_get_bool(s_config, ConfigKeyDateDisplayed)){
    const GColor date_color = config_get_color(s_config, ConfigKeyInfoColor);
    char buffer[] = "00";
    snprintf(buffer, sizeof(buffer), "%d", s_current_time->tm_mday);
    text_block_set_text(s_date_info, buffer, date_color);
  }
}

static void mark_dirty_minute_hand_layer(){
  layer_mark_dirty(s_minute_hand_layer);
  const bool rainbow_mode = config_get_bool(s_config, ConfigKeyRainbowMode);
  rot_bitmap_layer_set_angle(s_rainbow_hand_layer, angle_minute(s_current_time));
  layer_set_hidden((Layer*)s_rainbow_hand_layer, !rainbow_mode);
}


#define MINUTE_HAND_RADIUS 52

static void update_minute_hand_layer(Layer *layer, GContext * ctx){
  if(!config_get_bool(s_config, ConfigKeyRainbowMode)){
    const GPoint hand_end = gpoint_on_circle(s_center, angle_minute(s_current_time), MINUTE_HAND_RADIUS);
    graphics_context_set_stroke_width(ctx, 6);
    graphics_context_set_stroke_color(ctx, config_get_color(s_config, ConfigKeyMinuteHandColor));
    graphics_draw_line(ctx, s_center, hand_end);
  }
}

#define HOUR_HAND_RADIUS 39

static void update_hour_hand_layer(Layer * layer, GContext * ctx){
  const GPoint hand_end = gpoint_on_circle(s_center, angle_hour(s_current_time, true), HOUR_HAND_RADIUS);
  graphics_context_set_stroke_width(ctx, 6);
  graphics_context_set_stroke_color(ctx, config_get_color(s_config, ConfigKeyHourHandColor));
  graphics_draw_line(ctx, s_center, hand_end);
}

static void update_center_circle_layer(Layer * layer, GContext * ctx){
  GColor color = config_get_bool(s_config, ConfigKeyRainbowMode) ? GColorVividViolet : config_get_color(s_config, ConfigKeyHourHandColor);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_circle(ctx, s_center, 5);
}

// Ticks
static void draw_tick(GContext *ctx, const int index){
  graphics_draw_line(ctx, ticks_points[index][0], ticks_points[index][1]);
}

static void tick_layer_update_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, config_get_color(s_config, ConfigKeyTimeColor));
  graphics_context_set_stroke_width(ctx, 2);
  draw_tick(ctx, index_hour());
  if(!time_conflicts()){
    draw_tick(ctx, index_minute());
  }
}

// Infos: bluetooth + weather

static void update_weather_layer(){
  char info_buffer[6] = {0};
  const int timeout = (config_get_int(s_config, ConfigKeyRefreshRate) + 5) * 60;
  const int expiration =  s_weather.timestamp + timeout;
  const bool weather_valid = time(NULL) < expiration;
  if(weather_valid && config_get_bool(s_config, ConfigKeyWeatherEnabled)){
    int temp = s_weather.temperature;
    if(config_get_int(s_config, ConfigKeyTemperatureUnit) == Fahrenheit){
      temp = temp * 9 / 5 + 32;
    }
    char temp_buffer[6];
    snprintf(temp_buffer, 6, "%c%d°", s_weather.icon, temp);
    strcat(info_buffer, temp_buffer);
  }
  const GColor info_color = config_get_color(s_config, ConfigKeyInfoColor);
  text_block_set_text(s_weather_info, info_buffer, info_color);
}

static void update_watch_layer(){
  char info_buffer[4] = {0};
  if(s_battery_percent < 20){
    strncat(info_buffer, "w", 2);
  }
  const BluetoothIcon new_icon = config_get_int(s_config, ConfigKeyBluetoothIcon);
#ifndef NO_BT
  if(!s_bt_connected){
#endif
    if(new_icon == Bluetooth){
      strncat(info_buffer, "z", 2);
    }else if(new_icon == Heart){
      strncat(info_buffer, "Z", 2);
    }
#ifndef NO_BT
  }
#endif
  const GColor info_color = config_get_color(s_config, ConfigKeyInfoColor);
  text_block_set_text(s_watch_info, info_buffer, info_color);
}

static void bt_handler(bool connected){
  s_bt_connected = connected;
  update_watch_layer();
}

static void battery_handler(BatteryChargeState charge){
  s_battery_percent = charge.charge_percent;
  update_watch_layer();
}

static void step_handler(HealthEventType event, void *context){
  if(event == HealthEventSignificantUpdate){
    char step_text[8] = {0};
    const GColor info_color = config_get_color(s_config, ConfigKeyInfoColor);
    const int steps = (int)health_service_sum_today(HealthMetricStepCount);
    if(steps > 10000){
      snprintf(step_text, sizeof(step_text), "y%dk", steps / 1000);
    }else if(steps > 1000){
      snprintf(step_text, sizeof(step_text), "y%d.%dk", steps / 1000, (steps % 1000) / 100);
    }else{
      snprintf(step_text, sizeof(step_text), "y%d", steps);
    }
    text_block_set_text(s_steps_info, step_text, info_color);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  if(HOUR_UNIT & units_changed){
    bool vibrate_on_the_hour = config_get_bool(s_config, ConfigKeyVibrateOnTheHour);
    if (vibrate_on_the_hour) {
      if( PBL_IF_HEALTH_ELSE(true, false)  ||
          !(health_service_peek_current_activities() &
            (HealthActivitySleep | HealthActivityRestfulSleep)) ){
        vibes_short_pulse();
      }
    }
  }
  schedule_weather_request(10000);
  update_current_time();
  layer_mark_dirty(s_hour_hand_layer);
  mark_dirty_minute_hand_layer();
  update_times();
  update_date();
  layer_mark_dirty(s_tick_layer);
  step_handler(HealthEventSignificantUpdate, NULL);
  quadrants_update(s_quadrants, s_current_time);
}

static void main_window_load(Window *window) {
  s_root_layer = window_get_root_layer(window);
  s_root_layer_bounds = layer_get_bounds(s_root_layer);
  s_center = grect_center_point(&s_root_layer_bounds);
  update_current_time();
  window_set_background_color(window, config_get_color(s_config, ConfigKeyBackgroundColor));

  s_quadrants = quadrants_create(s_center, HOUR_HAND_RADIUS, MINUTE_HAND_RADIUS);
  s_date_info = quadrants_add_text_block(s_quadrants, s_root_layer, s_font, Low, s_current_time);
  text_block_set_enabled(s_date_info, config_get_bool(s_config, ConfigKeyDateDisplayed));
  
  s_steps_info = quadrants_add_text_block(s_quadrants, s_root_layer, s_font, High, s_current_time);
  health_service_events_subscribe(step_handler, NULL);
  step_handler(HealthEventSignificantUpdate, NULL);
  text_block_set_enabled(s_steps_info, config_get_bool(s_config, ConfigKeyHealthEnabled));

  s_weather_info = quadrants_add_text_block(s_quadrants, s_root_layer, s_font, Head, s_current_time);
  text_block_set_enabled(s_weather_info, config_get_bool(s_config, ConfigKeyWeatherEnabled));
  update_weather_layer();

  s_watch_info = quadrants_add_text_block(s_quadrants, s_root_layer, s_font, Tail, s_current_time);
  bluetooth_connection_service_subscribe(bt_handler);
  bt_handler(connection_service_peek_pebble_app_connection());
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  quadrants_ready(s_quadrants);
  quadrants_update(s_quadrants, s_current_time);

  s_hour_text = text_block_create(s_root_layer, time_points[6] , s_font);
  s_minute_text = text_block_create(s_root_layer, time_points[0] , s_font);

  s_tick_layer = layer_create(s_root_layer_bounds);
  layer_set_update_proc(s_tick_layer, tick_layer_update_callback);
  layer_add_child(s_root_layer, s_tick_layer);

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
  mark_dirty_minute_hand_layer();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_current_time();
  update_times();
  update_date();
  Message messages[] = {
    { AppKeyJsReady, js_ready_callback },
    { AppKeyBackgroundColor, config_background_color_updated },
    { AppKeyHourHandColor, config_hour_hand_color_updated },
    { AppKeyInfoColor, config_info_color_updated },
    { AppKeyMinuteHandColor, config_minute_hand_color_updated },
    { AppKeyTimeColor, config_time_color_updated },
    { AppKeyDateDisplayed, config_date_displayed_updated },
    { AppKeyRainbowMode, config_rainbow_mode_updated },
    { AppKeyBluetoothIcon, config_bluetooth_icon_updated },
    { AppKeyRefreshRate, config_refresh_rate_updated },
    { AppKeyTemperatureUnit, config_temperature_unit_updated },
    { AppKeyWeatherEnabled, config_weather_enabled_updated },
    { AppKeyWeatherTemperature, weather_requested_callback },
    { AppKeyVibrateOnTheHour, config_hourly_vibrate_updated },
    { AppKeyMilitaryTime, config_military_time_updated },
    { AppKeyHealthEnabled, config_health_enabled_updated }
  };
  s_messenger = messenger_create(16, messenger_callback, messages);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_hour_hand_layer);
  rot_bitmap_layer_destroy(s_rainbow_hand_layer);
  layer_destroy(s_minute_hand_layer);
  layer_destroy(s_center_circle_layer);
  gbitmap_destroy(s_rainbow_bitmap);

  text_block_destroy(s_hour_text);
  text_block_destroy(s_minute_text);

  layer_destroy(s_tick_layer);

  health_service_events_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  s_quadrants = quadrants_destroy(s_quadrants);

  text_block_destroy(s_weather_info);
  text_block_destroy(s_date_info);
  text_block_destroy(s_steps_info);
  text_block_destroy(s_watch_info);
}

static void init() {
  s_weather_request_timeout = 0;
  s_js_ready = false;
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NUPE_23));
  s_config = config_load(PersistKeyConfig, CONF_SIZE, CONF_DEFAULTS);
  if(persist_exists(PersistKeyWeather)){
    persist_read_data(PersistKeyWeather, &s_weather, sizeof(Weather));
  }
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  window_stack_remove(s_main_window, true);
  window_destroy(s_main_window);
  config_destroy(s_config);
  fonts_unload_custom_font(s_font);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
