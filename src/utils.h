#ifndef UTILS_H
#define UTILS_H

#include "uscript.h"

int32_t hash(value_t value);
value_t RawPair(state_t* S, type_t type, value_t left, value_t right);

#endif
