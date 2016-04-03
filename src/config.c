#include <pebble.h>
#include "config.h"
#include "common.h"

static Config * config_create(){
  Config *conf =  (Config *) malloc(sizeof(Config));
  *conf = (Config){
    .minute_hand_color = 0xffffff,
    .hour_hand_color   = 0xff0000,
    .background_color  = 0x000000,
    .date_color        = 0x555555,
    .time_color        = 0xAAAAAA,
    .info_color        = 0x555555,
    .date_displayed    = true,
    .bluetooth_icon    = Bluetooth,
    .temperature_unit  = Celsius,
    .rainbow_mode      = false,
    .weather_enabled   = true,
    .refresh_rate      = 20,
  };
  return conf;
}

bool config_get_bool(const Config * conf, const ConfigBoolKey key){
  switch(key){
  case ConfigBoolKeyRainbowMode:
    return conf->rainbow_mode;
  case ConfigBoolKeyDateDisplayed:
    return conf->date_displayed;
  default:
    return conf->weather_enabled;
  }
}

void config_set_bool(Config * conf, const ConfigBoolKey key, const bool value){
  switch(key){
  case ConfigBoolKeyRainbowMode:
    conf->rainbow_mode = value;
    break;
  case ConfigBoolKeyDateDisplayed:
    conf->date_displayed = value;
    break;
  default:
    conf->weather_enabled = value;
  }
}

GColor config_get_color(const Config * conf, const ConfigColorKey key){
  int color = 0;
  switch(key){
  case ConfigColorKeyMinuteHand:
    color = conf->minute_hand_color;
    break;
  case ConfigColorKeyHourHand:
    color = conf->hour_hand_color;
    break;
  case ConfigColorKeyBackground:
    color = conf->background_color;
    break;
  case ConfigColorKeyDate:
    color = conf->date_color;
    break;
  case ConfigColorKeyTime:
    color = conf->time_color;
    break;
  case ConfigColorKeyInfo:
    color = conf->info_color;
    break;
  }
  return GColorFromHEX(color);
}

void config_set_color(Config * conf, const ConfigColorKey key, const int value){
  switch(key){
  case ConfigColorKeyMinuteHand:
    conf->minute_hand_color = value;
    break;
  case ConfigColorKeyHourHand:
    conf->hour_hand_color = value;
    break;
  case ConfigColorKeyBackground:
    conf->background_color = value;
    break;
  case ConfigColorKeyDate:
    conf->date_color = value;
    break;
  case ConfigColorKeyTime:
    conf->time_color = value;
    break;
  case ConfigColorKeyInfo:
    conf->info_color = value;
    break;
  }
}

int config_get_int(const Config * conf, const ConfigIntKey key){
  switch(key){
  case ConfigIntKeyBluetoothIcon:
    return conf->bluetooth_icon;
  case ConfigIntKeyRefreshRate:
    return conf->refresh_rate;
  default:
    return conf->temperature_unit;
  }
}

void config_set_int(Config * conf, const ConfigIntKey key, const int value){
  switch(key){
  case ConfigIntKeyBluetoothIcon:
    conf->bluetooth_icon = value;
    break;
  case ConfigIntKeyRefreshRate:
    conf->refresh_rate = value;
    break;
  default:
    conf->temperature_unit = value;
  }
}

Config * config_load(const int persist_key){
  Config * conf = config_create();
  if(persist_exists(persist_key)){
    persist_read_data(persist_key, conf, sizeof(Config));
  }
  return conf;
}

void config_save(Config * conf, const int persist_key){
  persist_write_data(persist_key, conf, sizeof(Config));
}

Config * config_destroy(Config * conf){
  free(conf);
  return NULL;
}
