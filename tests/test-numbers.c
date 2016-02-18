#include "../src/uscript.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>

// state_t* State();
// void freeState(state_t* S);
//
// // NUMBERS
// value_t Char(int32_t code);
// value_t Bool(bool value);
// value_t Int(int32_t value);
// value_t Integer(state_t* S, int64_t value);
// int64_t toInt(state_t* S, value_t value);
// value_t Rational(state_t* S, int64_t n, int64_t d);

int main() {
  state_t* S = State();
  value_t v;

  v = Char('@');
  assert(v.type == CHARACTER);
  assert(v.value == '@');
  dump(S, v);

  v = Bool(true);
  assert(v.type == BOOLEAN);
  assert(v.value == true);
  dump(S, v);

  v = Bool(false);
  assert(v.type == BOOLEAN);
  assert(v.value == false);
  dump(S, v);

  v = Int(0);
  assert(v.type == INTEGER);
  assert(v.value == 0);
  dump(S, v);

  v = Int(2048);
  assert(v.type == INTEGER);
  assert(v.value == 2048);
  dump(S, v);

  v = Int(-2048);
  assert(v.type == INTEGER);
  assert(v.value == -2048);
  dump(S, v);

  v = Integer(S, 2048);
  assert(v.type == INTEGER);
  assert(v.value == 2048);
  dump(S, v);

  v = Integer(S, -2048);
  assert(v.type == INTEGER);
  assert(v.value == -2048);
  dump(S, v);

  v = Integer(S, 123456789);
  dump(S, v);
  assert(v.type == BOX_INTEGER);
  assert(toInt(S, v) == 123456789);

  v = Integer(S, -123456789);
  dump(S, v);
  assert(v.type == BOX_INTEGER);
  assert(toInt(S, v) == -123456789);

  freeState(S);
  return 0;
}
