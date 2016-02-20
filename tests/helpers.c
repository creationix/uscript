#include "../src/utils.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


#define BUFLEN 256
uint8_t buffer[BUFLEN];
void dump(state_t* S, value_t value) {
  uint8_t* end = buffer + BUFLEN;
  uint8_t* p = writeValue(buffer, end, S, value);
  printf("%.*s\n", (int)(p - buffer), buffer);
}
