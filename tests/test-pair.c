#include "../src/uscript.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>

// value_t Pair(state_t* S, value_t left, value_t right);
// pair_t getPair(state_t* S, value_t slot);
// value_t getLeft(state_t* S, value_t slot);
// value_t getRight(state_t* S, value_t slot);
// value_t setPair(state_t* S, value_t slot, value_t left, value_t right);
// value_t setLeft(state_t* S, value_t slot, value_t value);
// value_t setRight(state_t* S, value_t slot, value_t value);

int main() {
  state_t* S = State();
  value_t v, r;
  pair_t p;

  v = Pair(S, Char('a'), Char('b'));
  dump(S, v);
  assert(v.type == PAIR);
  p = getPair(S, v);
  assert(p.left.type == CHARACTER);
  assert(p.left.value == 'a');
  assert(p.right.type == CHARACTER);
  assert(p.right.value == 'b');
  r = getLeft(S, v);
  dump(S, r);
  assert(r.type == p.left.type);
  assert(r.value == p.left.value);
  r = getRight(S, v);
  dump(S, r);
  assert(r.type == p.right.type);
  assert(r.value == p.right.value);

  r = setLeft(S, v, Int(10));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == 1);
  p = getPair(S, v);
  assert(p.left.type == INTEGER);
  assert(p.left.value == 10);
  assert(p.right.type == CHARACTER);
  assert(p.right.value == 'b');

  r = setRight(S, v, Int(20));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);
  p = getPair(S, v);
  assert(p.left.type == INTEGER);
  assert(p.left.value == 10);
  assert(p.right.type == INTEGER);
  assert(p.right.value == 20);

  r = setPair(S, v, Char('A'), Char('B'));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);
  p = getPair(S, v);
  assert(p.left.type == CHARACTER);
  assert(p.left.value == 'A');
  assert(p.right.type == CHARACTER);
  assert(p.right.value == 'B');

  p = getPair(S, Int(100));
  dump(S, p.left);
  dump(S, p.right);
  assert(p.left.type == BOOLEAN);
  assert(p.left.value == false);
  assert(p.right.type == BOOLEAN);
  assert(p.right.value == false);

  r = getLeft(S, Int(200));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = getRight(S, Int(200));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setLeft(S, Int(200), Bool(true));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setRight(S, Int(200), Bool(false));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setPair(S, Char('_'), Char('Z'), Char('A'));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  freeState(S);
  return 0;
}
