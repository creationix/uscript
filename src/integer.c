#include "utils.h"
#include <stdlib.h>

value_t Char(int32_t code) {
  return (value_t){
    .gc = true,
    .type = CHARACTER,
    .value = code
  };
}

value_t Bool(bool value) {
  return (value_t){
    .gc = true,
    .type = BOOLEAN,
    .value = value
  };
}

value_t Int(int32_t value) {
  return (value_t){
    .gc = true,
    .type = INTEGER,
    .value = value & 0x7ffffff
  };
}

value_t Integer(state_t* S, int64_t value) {
  // If the value fits within the int27 range, store unboxed.
  if (value >= -0x4000000 && value < 0x4000000) {
    return Int((int32_t)value);
  }

  // Otherwise it must be stored in an int64 field.
  int32_t i = 0;
  while (true) {
    int64_t slot = S->ints[i];
    if (!slot) {
      S->ints[i] = value;
      break;
    }
    if (slot == value) break;
    i++;
    if (i < S->num_ints) {
      continue;
    }
    int32_t j = i + START_INTS;
    S->num_ints = j;
    S->ints = realloc(S->ints,  sizeof(int64_t) * (size_t)j);
    while (j-- > i) {
      S->ints[j] = 0;
    }
  }
  return (value_t){
    .gc = true,
    .type = BOX_INTEGER,
    .value = i
  };
}

int64_t toInt(state_t* S, value_t value) {
  switch (value.type) {
    case INTEGER: return value.value;
    case BOX_INTEGER: return S->ints[value.value];
    default: return 0; // TODO: should this thow an error?
  }
}
