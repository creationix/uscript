#ifndef UTILS_H
#define UTILS_H

#include "uscript.h"

typedef struct buffer_s {
  bool gc : 1;
  int length: 31;
  uint8_t data[];
} buffer_t;

bool eq(value_t left, value_t right);
bool falsy(value_t value);
bool truthy(value_t value);
bool isPair(value_t);
int32_t hash(value_t value);
buffer_t* getBuffer(state_t* S, value_t value);
value_t RawPair(state_t* S, type_t type, value_t left, value_t right);

#endif
