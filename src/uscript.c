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

static uint8_t* readInt(uint8_t* pc, int64_t* out) {
  *out = 0;
  int64_t extend = *pc & 0x40 ? ~0 : 0;
  do {
    extend <<= 7;
    *out = (*out << 7) | (*pc & 0x7f);
  } while (*pc++ & 0x80);
  *out |= extend;
  return pc;
}

uint8_t* eval(state_t* S, uint8_t* pc, value_t* out) {
  int64_t num, num2;
  switch ((opcode_t)*pc++) {
    case LitFalse:
      *out = Bool(false);
      return pc;
    case LitTrue:
      *out = Bool(true);
      return pc;
    case LitInt: // num
      pc = readInt(pc, &num);
      *out = Integer(S, num);
      return pc;
    case LitRational: // num num
      pc = readInt(pc, &num);
      pc = readInt(pc, &num2);
      *out = Rational(S, num, num2);
      return pc;
    case LitChar: // num
      pc = readInt(pc, &num);
      *out = Char((int32_t)num);
      return pc;
    case LitString: // len:num byte*
      pc = readInt(pc, &num);
      *out = String(S, (int32_t)num, pc);
      return pc + num;
    case LitSymbol: // len:num byte*
      pc = readInt(pc, &num);
      *out = Symbol(S, (int32_t)num, pc);
      return pc + num;
    case LitBuffer: // len:num byte*
      pc = readInt(pc, &num);
      *out = Buffer(S, (int32_t)num, pc);
      return pc + num;
    case LitPixels: // len:num byte*
      pc = readInt(pc, &num);
      *out = Pixels(S, (int32_t)num, (uint32_t*)pc);
      return pc + num * 4;
    case LitPair: { // value value
      value_t left, right;
      pc = eval(S, pc, &left);
      pc = eval(S, pc, &right);
      *out = Pair(S, left, right);
      return pc;
    }
    case LitStack: // len:num value*
      pc = readInt(pc, &num);
      *out = Stack(S);
      for (int i = 0; i < num; i++) {
        value_t value;
        pc = eval(S, pc, &value);
        stackPush(S, *out, value);
      }
      return pc;
    case LitSet: // len:num value*
      pc = readInt(pc, &num);
      *out = Set(S);
      for (int i = 0; i < num; i++) {
        value_t value;
        pc = eval(S, pc, &value);
        setAdd(S, *out, value);
      }
      return pc;
    case LitMap: // len:num (value/value)*
      pc = readInt(pc, &num);
      *out = Map(S);
      for (int i = 0; i < num; i++) {
        value_t key;
        value_t value;
        pc = eval(S, pc, &key);
        pc = eval(S, pc, &value);
        mapSet(S, *out, key, value);
      }
      return pc;
    case Block: // len:num value*
      pc = readInt(pc, &num);
      for (int i = 0; i < num; i++) {
        pc = eval(S, pc, out);
      }
      return pc;
  }
  return pc;
}
