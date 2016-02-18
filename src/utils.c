#include "utils.h"

bool eq(value_t left, value_t right) {
  return left.type == right.type && left.value == right.value;
}

bool falsy(value_t value) {
  return value.type == BOOLEAN && !value.value;
}

bool truthy(value_t value) {
  return !falsy(value);
}

bool isPair(value_t value) {
  return value.type == RATIONAL ||
         value.type == PAIR ||
         value.type == STACK ||
         value.type == SET ||
         value.type == MAP;
}

// Used by set and map. A 32-bit hash is generated for every value so that it
// takes a pseudo random path down the tree for fast search.
int32_t hash(value_t value) {
  return (value.type << 27 | value.value) ^ value.type ^ value.value << value.type;
}

buffer_t* getBuffer(state_t* S, value_t value) {
  return (buffer_t*)(S->bytes + value.value);
}
