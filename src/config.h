#pragma once

#include <stdint.h>

typedef struct {
  int32_t key;
  enum { ColorConf, IntConf, BoolConf } type;
  union {
    int32_t integer;
    int8_t boolean;
  } value;
} __attribute__((packed)) ConfValue;

typedef struct {
  ConfValue * data;
  int32_t size;
} Config;

int8_t config_get_bool(const Config * conf, const int32_t key);
void config_set_bool(Config * conf, const int32_t key, const int8_t value);
GColor config_get_color(const Config * conf, const int32_t key);
int32_t config_get_int(const Config * conf, const int32_t key);
void config_set_int(Config * conf, const int32_t key, const int32_t value);
Config * config_load(const int32_t persist_key, int32_t size, const ConfValue * defaults);
void config_save(Config * conf, const int32_t persist_key);
Config * config_destroy(Config * conf);
