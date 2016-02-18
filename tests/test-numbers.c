#include "../src/uscript.h"
#include "../src/dump.h"
#include <stdio.h>
#include <assert.h>

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
  assert(toInt(S, v) == 0);
  dump(S, v);

  v = Int(1);
  assert(v.type == INTEGER);
  assert(v.value == 1);
  assert(toInt(S, v) == 1);
  dump(S, v);

  v = Int(-1);
  assert(v.type == INTEGER);
  assert(v.value == -1);
  assert(toInt(S, v) == -1);
  dump(S, v);

  v = Int(2048);
  assert(v.type == INTEGER);
  assert(v.value == 2048);
  assert(toInt(S, v) == 2048);
  dump(S, v);

  v = Int(-2048);
  assert(v.type == INTEGER);
  assert(v.value == -2048);
  assert(toInt(S, v) == -2048);
  dump(S, v);

  v = Integer(S, 2048);
  assert(v.type == INTEGER);
  assert(v.value == 2048);
  assert(toInt(S, v) == 2048);
  dump(S, v);

  v = Integer(S, -2048);
  assert(v.type == INTEGER);
  assert(v.value == -2048);
  assert(toInt(S, v) == -2048);
  dump(S, v);

  v = Integer(S, 123456789);
  dump(S, v);
  assert(v.type == BOX_INTEGER);
  assert(toInt(S, v) == 123456789);

  v = Integer(S, -123456789);
  dump(S, v);
  assert(v.type == BOX_INTEGER);
  assert(toInt(S, v) == -123456789);

  v = Rational(S, 1, 2);
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 2);
  assert(toInt(S, v) == 0);

  v = Rational(S, 10, 3);
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 10);
  assert(toInt(S, getRight(S, v)) == 3);
  assert(toInt(S, v) == 3);

  v = Rational(S, 20, 6);
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 10);
  assert(toInt(S, getRight(S, v)) == 3);
  assert(toInt(S, v) == 3);

  v = Rational(S, 10, 2);
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 5);

  v = numberAdd(S, Int(1), Int(2));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 3);

  v = numberAdd(S, Rational(S, 1, 7), Rational(S, 2, 7));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 3);
  assert(toInt(S, getRight(S, v)) == 7);

  v = numberAdd(S, Rational(S, 5, 7), Rational(S, 9, 7));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberAdd(S, Rational(S, 1, 2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 5);
  assert(toInt(S, getRight(S, v)) == 6);

  v = numberSub(S, Int(1), Int(2));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == -1);

  v = numberSub(S, Rational(S, 1, 7), Rational(S, 2, 7));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == -1);
  assert(toInt(S, getRight(S, v)) == 7);

  v = numberSub(S, Rational(S, 17, 7), Rational(S, 3, 7));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberSub(S, Rational(S, 1, 2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 6);

  v = numberMul(S, Int(1), Int(2));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberMul(S, Rational(S, 2, 7), Rational(S, 2, 7));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 4);
  assert(toInt(S, getRight(S, v)) == 49);

  v = numberMul(S, Rational(S, 2, 3), Int(3));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberMul(S, Int(2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 2);
  assert(toInt(S, getRight(S, v)) == 3);

  v = numberDiv(S, Int(1), Int(2));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 2);

  v = numberDiv(S, Rational(S, 1, 7), Rational(S, 2, 7));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 2);

  v = numberDiv(S, Int(2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 6);

  v = numberDiv(S, Rational(S, 1, 2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 3);
  assert(toInt(S, getRight(S, v)) == 2);

  v = numberIDiv(S, Int(5), Int(2));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberIDiv(S, Rational(S, 23, 2), Rational(S, 9, 2));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);

  v = numberIDiv(S, Rational(S, 31, 3), Int(3));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 3);

  v = numberIDiv(S, Int(2), Rational(S, 1, 3));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 0);

  v = numberMod(S, Int(25), Int(10));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 5);

  v = numberMod(S, Rational(S, 1, 7), Rational(S, 2, 7));
  dump(S, v);
  assert(v.type == RATIONAL);
  assert(toInt(S, getLeft(S, v)) == 1);
  assert(toInt(S, getRight(S, v)) == 0);

  v = numberMod(S, Int(123), Rational(S, 100, 3));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 24);

  v = numberMod(S, Rational(S, 2, 1), Rational(S, 3, 1));
  dump(S, v);
  assert(v.type == INTEGER);
  assert(v.value == 2);


  freeState(S);
  return 0;
}
