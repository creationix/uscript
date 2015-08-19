#ifndef USCRIPT_VALUES_H
#define USCRIPT_VALUES_H

#include <stdint.h>

enum value_types {
  Bool,
  Integer,
  Buffer,
  Tuple,
};

struct value_s {
  enum value_types type;
  union {
    int32_t num;
    struct tuple_s *tuple;
    struct buffer_s *buffer;
  };
};

struct tuple_s {
  int top;
  int len;
  struct value_s values[];
};

struct buffer_s {
  int top;
  int len;
  uint8_t bytes[];
};

#endif
