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

  // logic (short circuit and/or)
  OP_NOT,
  OP_AND,
  OP_OR,

  // bitwise logic
  OP_BNOT,
  OP_BAND,
  OP_BOR,
  OP_BXOR,

  // comparison
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

static int32_t vars[26];

#define binop(code, op) \
  case code: { \
    int32_t a, b; \
    pc += eval(pc, &a); \
    pc += eval(pc, &b); \
    *res = a op b; \
    return pc - start; \
  }
#define unop(code, op) \
  case code: { \
    pc += eval(pc, res); \
    *res = op*res; \
    return pc - start; \
  }

size_t skip(const uint8_t* pc) {
  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    printf("SOP=%02x\n", *pc);

    switch (*pc++) {

    case OP_SET: return skip(pc + 1) + 2;
    case OP_GET: return 2;

    case OP_NOT:
    case OP_BNOT:
    case OP_NEG:
      return skip(pc) + 1;

    case OP_BAND:
    case OP_BOR:
    case OP_BXOR:
    case OP_EQ:
    case OP_NEQ:
    case OP_GTE:
    case OP_LT:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD: {
      const uint8_t* start = pc;
      pc += skip(pc);
      pc += skip(pc);
      return pc - start;
    }

    default:
      printf("Invalid Opcode\n");
      return -1;
    }
  }

  // Otherwise it's a variable length encoded integer.
  else {
    if (!(*pc++ & 0x40)) return 1;
    if (!(*pc++ & 0x80)) return 2;
    if (!(*pc++ & 0x80)) return 3;
    if (!(*pc++ & 0x80)) return 4;
    return 5;
  }
}

size_t eval(const uint8_t* pc, int32_t* res) {

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    const uint8_t* start = pc;
    printf("OP=%02x\n", *pc);

    switch (*pc++) {

    case OP_SET: {
      int idx = *pc++;
      pc += eval(pc, res);
      vars[idx] = *res;
      return pc - start;
    }
    case OP_GET: *res = vars[*pc]; return 2;

    unop(OP_NOT, !)
    case OP_AND:
      pc += eval(pc, res);
      if (*res) pc += eval(pc, res);
      else pc += skip(pc);
      return pc - start;
    case OP_OR:
      pc += eval(pc, res);
      if (*res) pc += skip(pc);
      else pc += eval(pc, res);
      return pc - start;

    unop(OP_BNOT, ~)
    binop(OP_BAND, &)
    binop(OP_BOR, |)
    binop(OP_BXOR, ^)

    binop(OP_EQ, ==)
    binop(OP_NEQ, !=)
    binop(OP_GTE, >=)
    binop(OP_LT, <)

    unop(OP_NEG, -)
    binop(OP_ADD, +)
    binop(OP_SUB, -)
    binop(OP_MUL, *)
    binop(OP_DIV, /)
    binop(OP_MOD, %)


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

static inline void test(const char* description, const uint8_t* program, int exp) {
  int32_t out;
  int used = eval(program, &out);
  printf("%s --> %d (%d/%d)\n", description, out, used, exp);
}

// Prints the bytes needed to encode a number.
// Useful for creating dummy programs.
void dump(uint32_t i) {
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

int main() {
  test("!42", (uint8_t[])
    { OP_NOT, 42 }, 2);
  test("7 || (b = -2)", (uint8_t[])
    { OP_OR, 7, OP_SET, 2, OP_NEG, 2 }, 6);
  test("b", (uint8_t[])
    { OP_GET, 2}, 2);
  test("7 && (b = -2)", (uint8_t[])
    { OP_AND, 7, OP_SET, 2, OP_NEG, 2 }, 6);
  test("b", (uint8_t[])
    { OP_GET, 2}, 2);
  test("~42", (uint8_t[])
    { OP_BNOT, 42 }, 2);
  test("43 & 34", (uint8_t[])
    { OP_BAND, 43, 34 }, 3);
  test("43 | 34", (uint8_t[])
    { OP_BOR, 43, 34 }, 3);
  test("43 ^ 34", (uint8_t[])
    { OP_BXOR, 43, 34 }, 3);
  test("a = 5", (uint8_t[])
    { OP_SET, 0, 5 }, 3);
  test("a", (uint8_t[])
    { OP_GET, 0 }, 2);
  test("1 < 2", (uint8_t[])
    { OP_LT, 1, 2 }, 3);
  test("1 >= 2", (uint8_t[])
    { OP_GTE, 1, 2 }, 3);
  test("1 == 2", (uint8_t[])
    { OP_EQ, 1, 2 }, 3);
  test("1 != 2", (uint8_t[])
    { OP_NEQ, 1, 2 }, 3);
  test("1 + (2 * 3)", (uint8_t[])
    { OP_ADD, 1, OP_MUL, 2, 3 }, 5);
  test("(1 - 2) * 3", (uint8_t[])
    { OP_MUL, OP_SUB, 1, 2, 3 }, 5);
  test("10 / 3", (uint8_t[])
    { OP_DIV, 10, 3 }, 3);
  test("10 % 3", (uint8_t[])
    { OP_MOD, 10, 3 }, 3);
  test("10", (uint8_t[])
    { 10 }, 1);
  test("100", (uint8_t[])
    { 100, 1 }, 2);
  test("1000", (uint8_t[])
    { 104, 15 }, 2);
  test("10000", (uint8_t[])
    { 80, 156, 1 }, 3);
  test("100000", (uint8_t[])
    { 96, 154, 12 }, 3);
  test("1000000", (uint8_t[])
    { 64, 137, 122 }, 3);
  test("10000000", (uint8_t[])
    { 64, 218, 196, 9 }, 4);
  test("100000000", (uint8_t[])
    { 64, 132, 175, 95 }, 4);
  test("1000000000", (uint8_t[])
    { 64, 168, 214, 185, 7 }, 5);
  test("-1000000000", (uint8_t[])
    { OP_NEG, 64, 168, 214, 185, 7 }, 6);
  return 0;
}
