#ifndef USCRIPT_VALUES_H
#define USCRIPT_VALUES_H

#include <stdint.h>

enum types_e {
  Bool,
  Integer,
  Buffer,
  Tuple,
};
typedef enum types_e types_t;

struct value_s {
  enum types_e type;
  union {
    int32_t num;
    struct tuple_s *tuple;
    struct buffer_s *buffer;
  };
};
typedef struct value_s value_t;

struct tuple_s {
  int ref;
  int len;
  value_t values[];
};
typedef struct tuple_s tuple_t;

struct buffer_s {
  int ref;
  int len;
  uint8_t bytes[];
};
typedef struct buffer_s buffer_t;

value_t new_bool(int val);
value_t new_integer(int val);
value_t new_buffer(int len);
value_t new_string(const char* str);
value_t new_tuple(int len);

int to_bool(value_t val);

value_t clone(value_t val);
#endif
