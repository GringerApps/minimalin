#include <pebble.h>
#include <stdlib.h>
#include "config.h"

static Config * config_create(const int32_t size){
  Config *conf =  (Config *) malloc(sizeof(Config));
  conf->data = (ConfValue *) malloc(size * sizeof(ConfValue));
  conf->size = size;
  return conf;
}

static ConfValue * value_for_key(const Config * conf, const int32_t key){
  for(int32_t i = 0; i < conf->size; i++){
    ConfValue * v = &conf->data[i];
    if(v->key == key){
      return v;
    }
  }
  return NULL;
}

int8_t config_get_bool(const Config * conf, const int32_t key){
  ConfValue * v = value_for_key(conf, key);
  if(v){
    return v->value.boolean;
  }
  return false;
}

void config_set_bool(Config * conf, const int32_t key, const int8_t value){
  ConfValue * v = value_for_key(conf, key);
  if(v){
    v->value.boolean = value;
  }
}

GColor config_get_color(const Config * conf, const int32_t key){
  return GColorFromHEX(config_get_int(conf, key));
}

int32_t config_get_int(const Config * conf, const int32_t key){
  ConfValue * v = value_for_key(conf, key);
  if(v){
    return v->value.integer;
  }
  return 0;
}

void config_set_int(Config * conf, const int32_t key, const int32_t value){
  ConfValue * v = value_for_key(conf, key);
  if(v){
    v->value.integer = value;
  }
}

Config * config_load(const int32_t persist_key, const int32_t size, const ConfValue * defaults){
  Config * conf = config_create(size);
  const int read_size = persist_read_data(persist_key, conf->data, size * sizeof(ConfValue));
  if(read_size == E_DOES_NOT_EXIST){
    memcpy(conf->data, defaults, size * sizeof(ConfValue));
  }else{
    const int config_size = read_size / sizeof(ConfValue);
    for(int i = config_size; i < size; i++){
      conf->data[i] = defaults[i];
    }
  }
  return conf;
}

void config_save(Config * conf, const int32_t persist_key){
  persist_write_data(persist_key, conf->data, conf->size * sizeof(ConfValue));
}

Config * config_destroy(Config * conf){
  free(conf->data);
  free(conf);
  return NULL;
}
