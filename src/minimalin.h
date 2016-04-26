#pragma once

typedef enum { NoIcon = 0, Bluetooth , Heart } BluetoothIcon;
typedef enum { Celsius = 0, Fahrenheit } TemperatureUnit;

typedef enum {
  ConfigKeyMinuteHandColor = 0,
  ConfigKeyHourHandColor,
  ConfigKeyBackgroundColor,
  ConfigKeyDateColor,
  ConfigKeyTimeColor,
  ConfigKeyInfoColor,
  ConfigKeyRefreshRate,
  ConfigKeyTemperatureUnit,
  ConfigKeyBluetoothIcon,
  ConfigKeyWeatherEnabled,
  ConfigKeyRainbowMode,
  ConfigKeyDateDisplayed
} ConfigKey;

#define CONF_SIZE 12

#if defined(PBL_COLOR)
ConfValue CONF_DEFAULTS[CONF_SIZE] = {
  { .key = ConfigKeyMinuteHandColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyHourHandColor, .type = ColorConf, .value = { .integer = 0xff0000 } },
  { .key = ConfigKeyBackgroundColor, .type = ColorConf, .value = { .integer = 0x000000 } },
  { .key = ConfigKeyDateColor, .type = ColorConf, .value = { .integer = 0x555555 } },
  { .key = ConfigKeyTimeColor, .type = ColorConf, .value = { .integer = 0xaaaaaa } },
  { .key = ConfigKeyInfoColor, .type = ColorConf, .value = { .integer = 0x555555 } },
  { .key = ConfigKeyBluetoothIcon, .type = IntConf, .value = { .integer = Bluetooth } },
  { .key = ConfigKeyTemperatureUnit, .type = IntConf, .value = { .integer = Celsius } },
  { .key = ConfigKeyRefreshRate, .type = IntConf, .value = { .integer = 20 } },
  { .key = ConfigKeyDateDisplayed, .type = BoolConf, .value = { .boolean = true } },
  { .key = ConfigKeyRainbowMode, .type = BoolConf, .value = { .boolean = false } },
  { .key = ConfigKeyWeatherEnabled, .type = BoolConf, .value = { .boolean = true } }
};
#elif defined(PBL_BW)
ConfValue CONF_DEFAULTS[CONF_SIZE] = {
  { .key = ConfigKeyMinuteHandColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyHourHandColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyBackgroundColor, .type = ColorConf, .value = { .integer = 0x000000 } },
  { .key = ConfigKeyDateColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyTimeColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyInfoColor, .type = ColorConf, .value = { .integer = 0xffffff } },
  { .key = ConfigKeyBluetoothIcon, .type = IntConf, .value = { .integer = Bluetooth } },
  { .key = ConfigKeyTemperatureUnit, .type = IntConf, .value = { .integer = Celsius } },
  { .key = ConfigKeyRefreshRate, .type = IntConf, .value = { .integer = 20 } },
  { .key = ConfigKeyDateDisplayed, .type = BoolConf, .value = { .boolean = true } },
  { .key = ConfigKeyRainbowMode, .type = BoolConf, .value = { .boolean = false } },
  { .key = ConfigKeyWeatherEnabled, .type = BoolConf, .value = { .boolean = true } }
};
#endif
