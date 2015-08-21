#include "values.h"
#include <stdlib.h>
#include <string.h>

value_t new_bool(int val) {
  return (value_t){ .type = Bool, .num = val };
}
value_t new_integer(int val) {
  return (value_t){ .type = Integer, .num = val };
}
value_t new_buffer(int len) {
  buffer_t* buffer = malloc(sizeof(buffer_t) + len * sizeof(uint8_t));
  buffer->ref = 1;
  buffer->len = len;
  memset(buffer->bytes, 0, len);
  return (value_t){ .type = Buffer, .buffer = buffer };
}

value_t new_string(const char* str) {
  int len = strlen(str);
  buffer_t* buffer = malloc(sizeof(buffer_t) + len * sizeof(uint8_t));
  buffer->ref = 1;
  buffer->len = len;
  memcpy(buffer->bytes, str, len);
  return (value_t){ .type = Buffer, .buffer = buffer };
}

value_t new_tuple(int len) {
  tuple_t* tuple = malloc(sizeof(tuple_t) + len * sizeof(value_t));
  tuple->ref = 1;
  tuple->len = len;
  return (value_t){ .type = Tuple, .tuple = tuple };
}

int to_bool(value_t val) {
  return val.type == Bool ? val.num : 0;
}


value_t clone(value_t val) {
  switch (val.type) {
    case Integer: case Bool:
      return val;
    case Buffer:
      val.buffer->ref++;
      return val;
    case Tuple:
      val.tuple->ref++;
      return val;
  }
}
