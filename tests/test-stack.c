#include "../src/uscript.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>

// value_t Stack(state_t* S);
// value_t stackPush(state_t* S, value_t stack, value_t value);
// value_t stackPeek(state_t* S, value_t stack);
// value_t stackLength(state_t* S, value_t stack);
// value_t stackPop(state_t* S, value_t stack);
// value_t stackIs(state_t* S, value_t left, value_t right);
// value_t stackReverse(state_t* S, value_t stack);

int main() {
  state_t* S = State();
  value_t v, r, c;

  v = Stack(S);
  dump(S, v);
  r = stackPush(S, v, Int(10));
  assert(toInt(S, r) == 1);
  dump(S, r);
  dump(S, v);

  r = stackPush(S, v, Int(20));
  assert(toInt(S, r) == 2);
  dump(S, r);
  dump(S, v);

  r = stackPush(S, v, Int(30));
  assert(toInt(S, r) == 3);
  dump(S, r);
  dump(S, v);

  c = stackReverse(S, v);
  dump(S, c);
  assert(c.type == STACK);

  r = stackIs(S, v, v);
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = stackIs(S, v, c);
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = stackIs(S, v, stackReverse(S, c));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = stackPeek(S, v);
  assert(toInt(S, r) == 30);
  dump(S, r);
  dump(S, v);

  r = stackLength(S, v);
  assert(toInt(S, r) == 3);
  dump(S, r);
  dump(S, v);

  r = stackPop(S, v);
  assert(toInt(S, r) == 30);
  dump(S, r);
  dump(S, v);

  r = stackIs(S, v, stackReverse(S, c));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = stackPop(S, v);
  assert(toInt(S, r) == 20);
  dump(S, r);
  dump(S, v);

  r = stackPop(S, v);
  assert(toInt(S, r) == 10);
  dump(S, r);
  dump(S, v);

  r = stackPeek(S, c);
  assert(toInt(S, r) == 10);
  dump(S, r);
  dump(S, c);

  r = stackLength(S, c);
  assert(toInt(S, r) == 3);
  dump(S, r);
  dump(S, c);

  r = stackPop(S, c);
  assert(toInt(S, r) == 10);
  dump(S, r);
  dump(S, c);

  r = stackIs(S, v, stackReverse(S, c));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = stackPop(S, c);
  assert(toInt(S, r) == 20);
  dump(S, r);
  dump(S, c);

  r = stackPop(S, c);
  assert(toInt(S, r) == 30);
  dump(S, r);
  dump(S, c);

  freeState(S);
  return 0;
}
