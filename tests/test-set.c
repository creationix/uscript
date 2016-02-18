#include "../src/uscript.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>

// value_t Set(state_t* S);
// value_t setAdd(state_t* S, value_t set, value_t value);
// value_t setHas(state_t* S, value_t set, value_t value);
// value_t setRemove(state_t* S, value_t set, value_t value);

int main() {
  state_t* S = State();
  value_t v, r;
  pair_t p;

  v = Set(S);
  dump(S, v);
  assert(v.type == SET);
  p = getPair(S, v);
  assert(p.left.type == BOOLEAN);
  assert(p.left.value == false);
  assert(p.right.type == BOOLEAN);
  assert(p.right.value == false);

  r = setAdd(S, v, Int(100));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setHas(S, v, Int(100));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setHas(S, v, Int(200));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setAdd(S, v, Int(200));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setHas(S, v, Int(100));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setHas(S, v, Int(200));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setAdd(S, v, Int(200));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setRemove(S, v, Int(100));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setHas(S, v, Int(100));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setHas(S, v, Int(200));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  r = setRemove(S, v, Int(100));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  r = setAdd(S, v, Int(100));
  dump(S, r);
  dump(S, v);
  assert(r.type == BOOLEAN);
  assert(r.value == true);

  for (int i = 0; i <= 100; i += 10) {
    r = setAdd(S, v, Int(i));
    dump(S, r);
    dump(S, v);
    assert(r.type == BOOLEAN);
    assert(r.value == (i < 100));
  }

  for (int i = 0; i <= 100; i += 5) {
    r = setHas(S, v, Int(i));
    dump(S, r);
    assert(r.type == BOOLEAN);
    assert(r.value == (i % 10 == 0));
    r = setRemove(S, v, Int(i));
    dump(S, r);
    dump(S, v);
    assert(r.type == BOOLEAN);
    assert(r.value == (i % 10 == 0));
    if (i % 10) {
      r = setAdd(S, v, Int(i));
      dump(S, r);
      dump(S, v);
      assert(r.type == BOOLEAN);
      assert(r.value == true);
      r = setHas(S, v, Int(i));
      dump(S, r);
      assert(r.type == BOOLEAN);
      assert(r.value == true);
    }
  }

  freeState(S);
  return 0;
}
