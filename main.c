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

size_t eval(uint8_t* pc, int32_t* res) {
  printf("OP=%02x\n", *pc);

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    uint8_t* start = pc;

    switch (*pc++) {

    case OP_ADD: {
      int32_t a, b;
      pc += eval(pc, &a);
      pc += eval(pc, &b);
      *res = a + b;
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
    *res |= (*pc & 0x7f) << 27;
    return 5;
  }
}

int main() {
  // add 1 2
  uint8_t program[] = {
    OP_ADD, 1, 2
  };
  int32_t out;
  eval(program, &out);

  printf("1 + 2 = %d\n", out);
  return 0;
}
