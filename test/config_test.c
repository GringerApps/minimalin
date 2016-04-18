#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <pebble.h>
#include <stdlib.h>
#include "config.h"

#define CONF_SIZE 3
const ConfValue CONF_DEFAULTS[CONF_SIZE] = {
  { .key = 0, .type = ColorConf, .value = 0xffffff },
  { .key = 1, .type = IntConf, .value = 20 },
  { .key = 2, .type = BoolConf, .value = true }
};

int __wrap_persist_write_data(const uint32_t key, const void * data, const size_t size){
  check_expected(key);
  check_expected_ptr(data);
  check_expected(size);
  return 1;
}

int __wrap_persist_read_data(const uint32_t key, void * buffer, const size_t buffer_size){
  int return_value = mock();
  if(return_value == E_DOES_NOT_EXIST){
    return return_value;
  }
  memcpy(buffer, mock_ptr_type(void *), return_value);
  return return_value;
}

GColor __wrap_GColorFromHEX(int hex){
  return (GColor ) { .hex = hex };
}

static void test_config_load_without_existing_config(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(0, CONF_SIZE, CONF_DEFAULTS);
  assert_int_equal(conf->size, CONF_SIZE);
  ConfValue first_value = conf->data[0];
  assert_int_equal(first_value.key, 0);
  assert_int_equal(first_value.type, ColorConf);
  assert_int_equal(first_value.value.integer, 0xffffff);
  ConfValue second_value = conf->data[1];
  assert_int_equal(second_value.key, 1);
  assert_int_equal(second_value.type, IntConf);
  assert_int_equal(second_value.value.integer, 20);
  ConfValue third_value = conf->data[2];
  assert_int_equal(third_value.key, 2);
  assert_int_equal(third_value.type, BoolConf);
  assert_true(third_value.value.boolean);

  config_destroy(conf);
}

static void test_config_load_with_existing_config(void **state){
  ConfValue persisted_value[CONF_SIZE] = {
    { .key = 2, .type = ColorConf, .value = 0x111111 },
    { .key = 1, .type = IntConf, .value = 10 },
    { .key = 0, .type = BoolConf, .value = false }
  };
  will_return(__wrap_persist_read_data, CONF_SIZE * sizeof(ConfValue));
  will_return(__wrap_persist_read_data, persisted_value);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);

  assert_int_equal(conf->size, CONF_SIZE);
  ConfValue first_value = conf->data[0];
  assert_int_equal(first_value.key, 2);
  assert_int_equal(first_value.type, ColorConf);
  assert_int_equal(first_value.value.integer, 0x111111);
  ConfValue second_value = conf->data[1];
  assert_int_equal(second_value.key, 1);
  assert_int_equal(second_value.type, IntConf);
  assert_int_equal(second_value.value.integer, 10);
  ConfValue third_value = conf->data[2];
  assert_int_equal(third_value.key, 0);
  assert_int_equal(third_value.type, BoolConf);
  assert_false(third_value.value.boolean);

  config_destroy(conf);
}

static void test_config_load_with_new_default(void **state){
  ConfValue persisted_value[CONF_SIZE] = {
    { .key = 2, .type = ColorConf, .value = 0x111111 },
    { .key = 1, .type = IntConf, .value = 10 },
    { .key = 0, .type = BoolConf, .value = false }
  };
  will_return(__wrap_persist_read_data, CONF_SIZE * sizeof(ConfValue));
  will_return(__wrap_persist_read_data, persisted_value);
  const ConfValue conf_with_extra_value[CONF_SIZE + 1] = {
    { .key = 0, .type = ColorConf, .value = 0xffffff },
    { .key = 1, .type = IntConf, .value = 20 },
    { .key = 2, .type = BoolConf, .value = true },
    { .key = 3, .type = IntConf, .value = 10 }
  };
  Config * conf = config_load(1, CONF_SIZE + 1, conf_with_extra_value);

  assert_int_equal(conf->size, CONF_SIZE);
  ConfValue first_value = conf->data[0];
  assert_int_equal(first_value.key, 2);
  assert_int_equal(first_value.type, ColorConf);
  assert_int_equal(first_value.value.integer, 0x111111);
  ConfValue second_value = conf->data[1];
  assert_int_equal(second_value.key, 1);
  assert_int_equal(second_value.type, IntConf);
  assert_int_equal(second_value.value.integer, 10);
  ConfValue third_value = conf->data[2];
  assert_int_equal(third_value.key, 0);
  assert_int_equal(third_value.type, BoolConf);
  assert_false(third_value.value.boolean);
  ConfValue fourth_value = conf->data[3];
  assert_int_equal(fourth_value.key, 3);
  assert_int_equal(fourth_value.type, IntConf);
  assert_int_equal(fourth_value.value.integer, 10);

  config_destroy(conf);
}

static void test_config_destroy(void **state){
  ConfValue persisted_value[CONF_SIZE] = {
    { .key = 2, .type = ColorConf, .value = 0x111111 },
    { .key = 1, .type = IntConf, .value = 10 },
    { .key = 0, .type = BoolConf, .value = false }
  };
  will_return(__wrap_persist_read_data, CONF_SIZE * sizeof(ConfValue));
  will_return(__wrap_persist_read_data, persisted_value);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);

  assert_null(config_destroy(conf));
}

static void test_config_set_int_incorrect_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);

  ConfValue first_value = conf->data[0];
  config_set_int(conf, 4, 3);
  assert_memory_equal(conf->data, CONF_DEFAULTS, CONF_SIZE * sizeof(ConfValue));

  config_destroy(conf);
}

static void test_config_set_int_correct_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  config_set_int(conf, 1, 3);
  ConfValue second_value = conf->data[1];
  assert_int_equal(second_value.key, 1);
  assert_int_equal(second_value.type, IntConf);
  assert_int_equal(second_value.value.integer, 3);
  config_destroy(conf);
}

static void test_config_get_int_incorrect_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  assert_int_equal(config_get_int(conf, 4), 0);
  config_destroy(conf);
}

static void test_config_get_int_correct_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  assert_int_equal(config_get_int(conf, 1), 20);
  config_destroy(conf);
}

static void test_config_get_color_incorrect_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  GColor color = config_get_color(conf, 4);
  assert_int_equal(color.hex, 0);
  config_destroy(conf);
}

static void test_config_get_color_correct_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  GColor color = config_get_color(conf, 0);
  assert_int_equal(color.hex, 0xffffff);
  config_destroy(conf);
}

static void test_config_set_bool_incorrect_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  ConfValue first_value = conf->data[2];
  config_set_bool(conf, 4, false);
  assert_memory_equal(conf->data, CONF_DEFAULTS, CONF_SIZE * sizeof(ConfValue));
  config_destroy(conf);
}

static void test_config_set_bool_correct_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  config_set_bool(conf, 2, false);
  ConfValue second_value = conf->data[2];
  assert_int_equal(second_value.key, 2);
  assert_int_equal(second_value.type, BoolConf);
  assert_false(second_value.value.boolean);
  config_destroy(conf);
}

static void test_config_get_bool_incorrect_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  assert_false(config_get_bool(conf, 4));
  config_destroy(conf);
}

static void test_config_get_bool_correct_key(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  assert_false(config_get_bool(conf, 3));
  config_destroy(conf);
}

static void test_config_save(void **state){
  will_return(__wrap_persist_read_data, E_DOES_NOT_EXIST);
  Config * conf = config_load(1, CONF_SIZE, CONF_DEFAULTS);
  config_set_int(conf, 1, 5);
  config_set_bool(conf, 2, false);
  expect_value(__wrap_persist_write_data, key, 1);
  int data_size = CONF_SIZE * sizeof(ConfValue);
  expect_memory(__wrap_persist_write_data, data, conf->data, data_size);
  expect_value(__wrap_persist_write_data, size, data_size);
  config_save(conf, 1);
  config_destroy(conf);
}

int main(void){
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_config_load_without_existing_config),
    cmocka_unit_test(test_config_load_with_existing_config),
    cmocka_unit_test(test_config_destroy),
    cmocka_unit_test(test_config_set_int_incorrect_key),
    cmocka_unit_test(test_config_set_int_correct_key),
    cmocka_unit_test(test_config_get_int_correct_key),
    cmocka_unit_test(test_config_get_int_incorrect_key),
    cmocka_unit_test(test_config_get_color_correct_key),
    cmocka_unit_test(test_config_get_color_incorrect_key),
    cmocka_unit_test(test_config_set_bool_incorrect_key),
    cmocka_unit_test(test_config_set_bool_correct_key),
    cmocka_unit_test(test_config_get_bool_correct_key),
    cmocka_unit_test(test_config_get_bool_incorrect_key),
    cmocka_unit_test(test_config_save),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
