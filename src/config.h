#pragma once

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

typedef enum {
  ConfigColorKeyMinuteHand,
  ConfigColorKeyHourHand,
  ConfigColorKeyBackground,
  ConfigColorKeyDate,
  ConfigColorKeyTime,
  ConfigColorKeyInfo
} ConfigColorKey;

typedef enum {
  ConfigBoolKeyWeatherEnabled,
  ConfigBoolKeyRainbowMode,
  ConfigBoolKeyDateDisplayed
} ConfigBoolKey;

typedef enum {
  ConfigIntKeyRefreshRate,
  ConfigIntKeyTemperatureUnit,
  ConfigIntKeyBluetoothIcon
} ConfigIntKey;

bool config_get_bool(const Config * conf, const ConfigBoolKey key);
void config_set_bool(Config * conf, const ConfigBoolKey key, const bool value);
GColor config_get_color(const Config * conf, const ConfigColorKey key);
void config_set_color(Config * conf, const ConfigColorKey key, const int value);
int config_get_int(const Config * conf, const ConfigIntKey key);
void config_set_int(Config * conf, const ConfigIntKey key, const int value);
Config * config_load(const int persist_key);
void config_save(Config * conf, const int persist_key);
Config * config_destroy(Config * conf);
