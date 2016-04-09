#include <sys/types.h>
#include <string.h>
#include <stdint.h>


typedef enum { false, true } bool;
typedef enum { E_DOES_NOT_EXIST } StatusCode;

typedef struct {
  int hex;
} GColor;

GColor GColorFromHEX(const int hex);
int persist_write_data(const uint32_t key, const void * data, const size_t size);
bool persist_exists(const uint32_t key);
int persist_read_data(const uint32_t key, void * buffer, const size_t buffer_size);
