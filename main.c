#include <stdio.h>
#include <stdint.h>

enum opcodes {

  // user code
  OP_DEF = 128,
  OP_RM,
  OP_CALL,
  OP_RUN,
  OP_RETURN,

  // variables
  OP_SET,
  OP_GET,

  // control
  OP_IF,
  OP_MATCH,
  OP_WHILE,
  OP_DO,
  OP_LOOP,
  OP_BREAK,

  // logic
  OP_NOT,
  OP_AND,
  OP_OR,
  OP_XOR,
  OP_EQ,
  OP_NEQ,
  OP_GTE,
  OP_LT,

  // math
  OP_NEG,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_RAND,

  // events
  OP_WAIT,
  OP_ON,

  // timer
  OP_DELAY,
  OP_TIMER,

  // io
  OP_PM,
  OP_DW,
  OP_PW,
  OP_DR,
  OP_AR,

  // neopixel
  OP_NP,
  OP_NPW,
  OP_RGB,
  OP_HSV,
  OP_HCL,
  OP_UPDATE,

  // servo
  OP_SERVO,
  OP_MOVE,

  // tone
  OP_TONE,
};

size_t eval(const uint8_t* pc, int32_t* res) {

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    const uint8_t* start = pc;
    printf("OP=%02x\n", *pc);

    switch (*pc++) {

    case OP_ADD: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a + b;
      return pc - start;
    }

    case OP_SUB: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a - b;
      return pc - start;
    }

    case OP_MUL: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a * b;
      return pc - start;
    }

    case OP_DIV: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a / b;
      return pc - start;
    }

    case OP_MOD: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a % b;
      return pc - start;
    }

    default:
      printf("Invalid Opcode\n");
      return -1;
    }
  }

  // Otherwise it's a variable length encoded integer.
  else {
    *res = *pc & 0x3f;
    if (!(*pc++ & 0x40)) return 1;
    *res |= (*pc & 0x7f) << 6;
    if (!(*pc++ & 0x80)) return 2;
    *res |= (*pc & 0x7f) << 13;
    if (!(*pc++ & 0x80)) return 3;
    *res |= (*pc & 0x7f) << 20;
    if (!(*pc++ & 0x80)) return 4;
    *res |= (*pc & 0x1f) << 27;
    return 5;
  }
}

static inline void test(const char* description, const uint8_t* program) {
  int32_t out;
  eval(program, &out);
  printf("%s = %d\n", description, out);
}

// Prints the bytes needed to encode a number.
// Useful for creating dummy programs.
void dump(uint32_t i) {
  if (i < 0x40) {
    printf("%d\n", i & 0x3f);
    return;
  }
  printf("%d ", (i & 0x3f) | 0x40);
  i >>= 6;
  while (i) {
    if (i < 0x80) {
      printf("%d\n", i & 0x7f);
      return;
    }
    printf("%d ", (i & 0x7f) | 0x80);
    i >>= 7;
  }
}
int main() {
  test("1 + (2 * 3)", (uint8_t[])
    { OP_ADD, 1, OP_MUL, 2, 3 });
  test("(1 + 2) * 3", (uint8_t[])
    { OP_MUL, OP_ADD, 1, 2, 3 });
  test("10 / 3", (uint8_t[])
    { OP_DIV, 10, 3 });
  test("10 % 3", (uint8_t[])
    { OP_MOD, 10, 3 });
  test("10", (uint8_t[])
    { 10 });
  test("100", (uint8_t[])
    { 100, 1 });
  test("1000", (uint8_t[])
    { 104, 15 });
  test("10000", (uint8_t[])
    { 80, 156, 1 });
  test("100000", (uint8_t[])
    { 96, 154, 12 });
  test("1000000", (uint8_t[])
    { 64, 137, 122 });
  test("10000000", (uint8_t[])
    { 64, 218, 196, 9 });
  test("100000000", (uint8_t[])
    { 64, 132, 175, 95 });
  test("1000000000", (uint8_t[])
    { 64, 168, 214, 185, 7 });
  return 0;
}
