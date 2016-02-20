#ifndef DUMP_H
#define DUMP_H

#include "utils.h"

uint8_t* writeValue(uint8_t* start, uint8_t* end, state_t* S, value_t value);
void dump(state_t* S, value_t value);

#endif
