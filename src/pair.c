#include "utils.h"
#include <string.h>
#include <stdlib.h>

value_t RawPair(state_t* S, type_t type, value_t left, value_t right) {
  // Find an empty slot in the pairs table
  int32_t i = S->next_pair;
  while (true) {
    if (i >= S->num_pairs) {
      // TODO: gc
      int32_t old_length = S->num_pairs;
      int32_t new_length = old_length + START_PAIRS;
      size_t old_size = (size_t)old_length * sizeof(pair_t);
      size_t new_size = (size_t)new_length * sizeof(pair_t);
      S->pairs = realloc(S->pairs, new_size);
      memset((uint8_t*)S->pairs + old_size, 0, new_size - old_size);
      S->num_pairs = new_length;
      continue;
    }
    pair_t slot = S->pairs[i];
    if (!(slot.left.gc || slot.right.gc)) {
      // If both GC flags are zero, assume it's free.
      break;
    }
    ++i;
  }
  S->next_pair = i + 1;
  S->pairs[i] = (pair_t){
    .left =  left,
    .right = right
  };
  return (value_t){
    .gc = true,
    .type = type,
    .value = i
  };
}

value_t Pair(state_t* S, value_t left, value_t right) {
  return RawPair(S, PAIR, left, right);
}

pair_t getPair(state_t* S, value_t slot) {
  if (!isPair(slot)) return (pair_t) {
    .left = Bool(false),
    .right = Bool(false)
  };
  return S->pairs[slot.value];
}

void setPair(state_t* S, value_t slot, value_t left, value_t right) {
  if (!isPair(slot)) return;
  S->pairs[slot.value] = (pair_t) {
    .left = left,
    .right = right
  };
}

void setLeft(state_t* S, value_t slot, value_t value) {
  if (!isPair(slot)) return;
  S->pairs[slot.value] = (pair_t){
    .left = value,
    .right = S->pairs[slot.value].right
  };
}

void setRight(state_t* S, value_t slot, value_t value) {
  if (!isPair(slot)) return;
  S->pairs[slot.value] = (pair_t){
    .left = S->pairs[slot.value].left,
    .right = value
  };
}

value_t getLeft(state_t* S, value_t slot) {
  if (!isPair(slot)) return Bool(false);
  return S->pairs[slot.value].left;
}

value_t getRight(state_t* S, value_t slot) {
  if (!isPair(slot)) return Bool(false);
  return S->pairs[slot.value].right;
}
