#include "helpers.c"

// value_t Map(state_t* S);
// value_t mapSet(state_t* S, value_t map, value_t key, value_t value);
// value_t mapDelete(state_t* S, value_t map, value_t key);
// value_t MapGet(state_t* S, value_t map, value_t key);

int main() {
  state_t* S = State();
  value_t v, r;

  v = Map(S);
  dump(S, v);
  assert(v.type == MAP);
  assert(isPair(v));

  value_t name = Symbol(S, -1, (const uint8_t*)"name");
  dump(S, name);

  value_t age = Symbol(S, -1, (const uint8_t*)"age");
  dump(S, age);

  r = mapSet(S, v, name, String(S, -1, (const uint8_t*)"Tim"));
  dump(S, r);
  dump(S, v);
  assert(r.value == true);

  r = mapSet(S, v, age, Int(33));
  dump(S, r);
  dump(S, v);
  assert(r.value == true);

  r = mapSet(S, v, age, Int(33));
  dump(S, r);
  dump(S, v);
  assert(r.value == false);

  r = mapGet(S, v, name);
  dump(S, r);
  assert(r.type == STRING);

  r = mapGet(S, v, age);
  dump(S, r);
  assert(r.type == INTEGER);

  r = mapGet(S, v, Char('a'));
  dump(S, r);
  assert(r.type == BOOLEAN);
  assert(r.value == false);

  for (int i = 0; i <= 100; i += 10) {
    r = mapSet(S, v, Int(i), Int(i*2));
    dump(S, r);
    dump(S, v);
    assert(r.type == BOOLEAN);
    assert(r.value == true);
  }

  for (int i = 0; i <= 100; i += 5) {
    r = mapGet(S, v, Int(i));
    dump(S, r);
    if (i % 10) {
      assert(r.type == BOOLEAN);
      assert(r.value == false);
    }
    else {
      assert(r.type == INTEGER);
      assert(r.value == i * 2);
    }
    r = mapDelete(S, v, Int(i));
    dump(S, r);
    dump(S, v);
    assert(r.type == BOOLEAN);
    assert(r.value == (i % 10 == 0));
    if (i % 10) {
      r = mapSet(S, v, Int(i),Int(i*3));
      dump(S, r);
      dump(S, v);
      assert(r.type == BOOLEAN);
      assert(r.value == true);
      r = mapGet(S, v, Int(i));
      dump(S, r);
      assert(r.type == INTEGER);
      assert(r.value == i*3);
    }
  }

  freeState(S);
  return 0;
}
