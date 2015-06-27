//#define OP_LOG
#include "uscript.c"
#include <stdio.h>

static inline void test(const char* description, const uint8_t* program, int32_t exp, int32_t res) {
  int32_t out;
  int32_t used = eval(program, &out) - program;
  printf("%s --> %d (%d/%d)\n", description, out, used, exp);
  assert(used == exp);
  assert(res == out);
}

#if 0
// Prints the bytes needed to encode a number.
// Useful for creating dummy programs.
static void dump(uint32_t i) {
  if (i < 0x40) {
    printf("%d\n", i & 0x3f);
    return;
  }
  printf("%d, ", (i & 0x3f) | 0x40);
  i >>= 6;
  while (i) {
    if (i < 0x80) {
      printf("%d\n", i & 0x7f);
      return;
    }
    printf("%d, ", (i & 0x7f) | 0x80);
    i >>= 7;
  }
}
#endif

int main() {
  test("!42", (uint8_t[])
    { OP_NOT, 42 }, 2, 0);
  test("7 || (b = -2)", (uint8_t[])
    { OP_OR, 7, OP_SET, 2, OP_NEG, 2 }, 6, 7);
  test("b", (uint8_t[])
    { OP_GET, 2}, 2, 0);
  test("7 && (b = -2)", (uint8_t[])
    { OP_AND, 7, OP_SET, 2, OP_NEG, 2 }, 6, -2);
  test("b", (uint8_t[])
    { OP_GET, 2}, 2, -2);
  test("~42", (uint8_t[])
    { OP_BNOT, 42 }, 2, -43);
  test("43 & 34", (uint8_t[])
    { OP_BAND, 43, 34 }, 3, 34);
  test("43 | 34", (uint8_t[])
    { OP_BOR, 43, 34 }, 3, 43);
  test("43 ^ 34", (uint8_t[])
    { OP_BXOR, 43, 34 }, 3, 9);
  test("a = 5", (uint8_t[])
    { OP_SET, 0, 5 }, 3, 5);
  test("a", (uint8_t[])
    { OP_GET, 0 }, 2, 5);
  test("1 < 2", (uint8_t[])
    { OP_LT, 1, 2 }, 3, 1);
  test("1 >= 2", (uint8_t[])
    { OP_GTE, 1, 2 }, 3, 0);
  test("1 == 2", (uint8_t[])
    { OP_EQ, 1, 2 }, 3, 0);
  test("1 != 2", (uint8_t[])
    { OP_NEQ, 1, 2 }, 3, 1);
  test("1 + (2 * 3)", (uint8_t[])
    { OP_ADD, 1, OP_MUL, 2, 3 }, 5, 7);
  test("(1 - 2) * 3", (uint8_t[])
    { OP_MUL, OP_SUB, 1, 2, 3 }, 5, -3);
  test("10 / 3", (uint8_t[])
    { OP_DIV, 10, 3 }, 3, 3);
  test("10 % 3", (uint8_t[])
    { OP_MOD, 10, 3 }, 3, 1);
  test("10", (uint8_t[])
    { 10 }, 1, 10);
  test("100", (uint8_t[])
    { 100, 1 }, 2, 100);
  test("1000", (uint8_t[])
    { 104, 15 }, 2, 1000);
  test("10000", (uint8_t[])
    { 80, 156, 1 }, 3, 10000);
  test("100000", (uint8_t[])
    { 96, 154, 12 }, 3, 100000);
  test("1000000", (uint8_t[])
    { 64, 137, 122 }, 3, 1000000);
  test("10000000", (uint8_t[])
    { 64, 218, 196, 9 }, 4, 10000000);
  test("100000000", (uint8_t[])
    { 64, 132, 175, 95 }, 4, 100000000);
  test("1000000000", (uint8_t[])
    { 64, 168, 214, 185, 7 }, 5, 1000000000);
  test("-1000000000", (uint8_t[])
    { OP_NEG, 64, 168, 214, 185, 7 }, 6, -1000000000);
  test("if (1) 9", (uint8_t[])
    { OP_IF, 1, 9 }, 3, 9);
  test("if (0) 9", (uint8_t[])
    { OP_IF, 0, 9 }, 3, 0);
  test("if (1) 9 else 5", (uint8_t[])
    { OP_IF, 1, 9, OP_ELSE, 5}, 5, 9);
  test("if (0) 9 else 5", (uint8_t[])
    { OP_IF, 0, 9, OP_ELSE, 5}, 5, 5);
  test("if (1) 9 elif (0) 3 else 5", (uint8_t[])
    { OP_IF, 1, 9, OP_ELIF, 0, 3, OP_ELSE, 5}, 8, 9);
  test("if (0) 9 elif (1) 3 else 5", (uint8_t[])
    { OP_IF, 0, 9, OP_ELIF, 1, 3, OP_ELSE, 5}, 8, 3);
  test("match (42) with (42) 7", (uint8_t[])
    { OP_MATCH, 42, OP_WHEN, 42, 7 }, 5, 7);
  test("match (42) with (34) 7", (uint8_t[])
    { OP_MATCH, 42, OP_WHEN, 34, 7 }, 5, 0);
  test("match (42) with (34) 7 with (42) 9", (uint8_t[])
    { OP_MATCH, 42, OP_WHEN, 34, 7, OP_WHEN, 42, 9 }, 8, 9);
  test("match (42) with (34) 7 else 5", (uint8_t[])
    { OP_MATCH, 42, OP_WHEN, 34, 7, OP_ELSE, 5}, 7, 5);
  test("i = 10", (uint8_t[])
    { OP_SET, 8, 10 }, 3, 10);
  test("while (i) --i", (uint8_t[])
    { OP_WHILE, OP_GET, 8, OP_DECR, 8 }, 5, 0);
  test("s = 0", (uint8_t[])
    { OP_SET, 18, 0 }, 3, 0);
  test("while (i < 10) do { ++i; s = s + i; }", (uint8_t[])
    { OP_WHILE, OP_LT, OP_GET, 8, 10, OP_DO, 2,
        OP_INCR, 8,
        OP_SET, 18, OP_ADD, OP_GET, 18, OP_GET, 8 }, 16, 55);
  test("while (--i) s = s + i", (uint8_t[])
    { OP_WHILE, OP_DECR, 8,
        OP_SET, 18, OP_ADD, OP_GET, 18, OP_GET, 8 }, 10, 100);
  test("while 1 delay 1000", (uint8_t[])
    { OP_WHILE, 1, OP_DELAY, 104, 15 }, 5, 1000);
  return 0;
}
