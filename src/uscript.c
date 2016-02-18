#include "uscript.h"
#include <stdlib.h>
#include <string.h>

state_t* State() {
  state_t* S = malloc(sizeof(state_t));
  S->num_pairs = START_PAIRS;
  S->next_pair = 0;
  S->pairs = calloc(START_PAIRS, sizeof(pair_t));
  S->num_bytes = START_BYTES;
  S->bytes = calloc(START_BYTES, sizeof(uint8_t));
  S->num_ints = START_INTS;
  S->ints = calloc(START_INTS, sizeof(int64_t));
  return S;
}

void freeState(state_t* S) {
  free(S->ints);
  free(S->bytes);
  free(S->pairs);
  memset(S, 0, sizeof(state_t));
}
