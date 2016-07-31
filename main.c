#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef enum {
  // Integers
  // 0-63 and 192-255 is literal int7_t
  INT8 = 64, // next 1 byte is int8_t
  INT16,      // next 2 bytes are int16_t (LE)
  INT32,      // next 4 bytes are int32_t (LE)

  // Control Flow
  FOREVER, // (block)
  WHILE,   // (cond, block)
  IF,      // (cond, block)
  ELIF,    // (cond, block)
  ELSE,    // (block)
  FOR,     // (start, stop, incr, slot, block)

  // Variables
  SET,     // (slot, expr)
  GET,     // (slot)
  GSET,    // (gslot, expr)
  GGET,    // (gslot)

  // User Functions
  DEF,     // (fslot, block)
  CALL,    // (fslot)
  CALL1,   // (fslot, expr)
  CALL2,   // (fslot, expr, expr)
  CALL3,   // (fslot, expr, expr, expr)
  CALL4,   // (fslot, expr, expr, expr, expr)

  // Non-local jumps
  RETURN,  // (expr) Early return
  BREAK,   // (expr) Exit current loop
  CONTINUE,// Start loop over

  // Math
  ADD,     // (expr, expr)
  SUB,     // (expr, expr)
  MUL,     // (expr, expr)
  DIV,     // (expr, expr)
  MOD,     // (expr, expr)
  NEG,     // (expr)

  // Bitwise Math
  BAND,    // (expr, expr)
  BOR,     // (expr, expr)
  BXOR,    // (expr, expr)
  BNOT,    // (expr)
  LSHIFT,  // (expr, expr)
  RSHIFT,  // (expr, expr)

  // Comparison
  GT,      // (expr, expr)
  GTE,     // (expr, expr)
  LT,      // (expr, expr)
  LTE,     // (expr, expr)
  EQ,      // (expr, expr)
  NEQ,     // (expr, expr)

  // Logic
  AND,     // (expr, expr)
  OR,      // (expr, expr)
  XOR,     // (expr, expr)
  NOT,     // (expr)

  DO, END, // variable length block

  // 191 back to END + 1 is for custom functions

  // 0-63 and 192-255 is literal int7_t
} opcode_t;



typedef struct {
  union {
    int32_t (*fn0)();
    int32_t (*fn1)(int32_t arg1);
    int32_t (*fn2)(int32_t arg1, int32_t arg2);
    int32_t (*fn3)(int32_t arg1, int32_t arg2, int32_t arg3);
    int32_t (*fn4)(int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4);
  };
  uint8_t len;
} fn_t;

int pinMode(int pin, int mode) {
  printf("pinMode(%d, %d)\n", pin, mode);
  return mode;
}

int digitalWrite(int pin, int value) {
  printf("digitalWrite(%d, %d)\n", pin, value);
  return value;
}

int digitalRead(int pin) {
  printf("digitalRead(%d)\n", pin);
  return 0;
}

int delay(int ms) {
  printf("delay(%d)\n", ms);
  return ms;
}

int32_t stack[64];
int offset;
int highest;

uint8_t *user[32];

fn_t funcs[] = {
  {.fn2=pinMode,.len=2},
  {.fn2=digitalWrite,.len=2},
  {.fn1=digitalRead,.len=1},
  {.fn1=delay,.len=1},
};

enum user {
  MODE = END+1,
  DW,
  DR,
  DELAY,
};

uint8_t* eval(uint8_t *code, int32_t *value);
uint8_t* skip(uint8_t *code);

uint8_t* skipNative(uint8_t *code, int slot) {
  fn_t *fn = funcs + slot;
  switch (fn->len) {
    case 0: return code;
    case 1: return skip(code);
    case 2: return skip(skip(code));
    case 3: return skip(skip(skip(code)));
    case 4: return skip(skip(skip(skip(code))));
  }
  return code;
}

uint8_t* callNative(uint8_t *code, int slot, int32_t *value) {
  if (!value) return skipNative(code, slot);
  fn_t *fn = funcs + slot;
  switch (fn->len) {
    case 0: {
      *value = fn->fn0();
      return code;
    }
    case 1: {
      int32_t arg0;
      code = eval(code, &arg0);
      *value = fn->fn1(arg0);
      return code;
    }
    case 2: {
      int32_t arg0, arg1;
      code = eval(eval(code, &arg0), &arg1);
      *value = fn->fn2(arg0, arg1);
      return code;
    }
    case 3: {
      int32_t arg0, arg1, arg2;
      code = eval(eval(eval(code, &arg0), &arg1), &arg2);
      *value = fn->fn3(arg0, arg1, arg2);
      return code;
    }
    case 4: {
      int32_t arg0, arg1, arg2, arg3;
      code = eval(eval(eval(eval(code, &arg0), &arg1), &arg2), &arg3);
      *value = fn->fn4(arg0, arg1, arg2, arg3);
      return code;
    }
  }
  return code;
}

uint8_t* skip(uint8_t *code) {
  if (!code) return code;
  if (*code < 64 || *code >= 192) return code + 1;
  if (*code > END) {
    return callNative(code + 1, *code - END - 1, NULL);
  }
  switch ((opcode_t)*code++) {
    case INT8: return code + 1;
    case INT16: return code + 2;
    case INT32: return code + 4;

    // Consume zero expressions
    case CONTINUE:
      return code;

    // Consume one expression
    case FOREVER:
    case GET: case GGET:
    case CALL:
    case RETURN: case BREAK:
    case NEG: case NOT: case BNOT:
      return skip(code);

    // Consume two expressions
    case WHILE:
    case SET: case GSET:
    case DEF:
    case CALL1:
    case ADD: case SUB: case MUL: case DIV: case MOD:
    case BAND: case BOR: case BXOR: case LSHIFT: case RSHIFT:
    case GT: case GTE: case LT: case LTE: case EQ: case NEQ:
    case AND: case OR: case XOR:
      return skip(skip(code));

    // Consume three expressions
    case CALL2:
      return skip(skip(skip(code)));

    // Consume four expressions
    case FOR:
    case CALL3:
      return skip(skip(skip(skip(code))));

    // Consume five expressions
    case CALL4:
      return skip(skip(skip(skip(skip(code)))));

    case IF:
      code = skip(skip(code));
      while (*code == ELIF) {
        code = skip(skip(++code));
      }
      if (*code == ELSE) {
        code = skip(++code);
      }
      return code;

    case DO:
      while (*code != END) {
        code = skip(code);
      }
      return code + 1;

    // Invalid expression starters
    case ELIF: case ELSE: case END:
      return code;
  }
  return code;
}

uint8_t* eval(uint8_t *code, int32_t *value) {
  if (!code) return code;
  if (!value) return skip(code);
  if (*code < 64 || *code >= 192) {
    *value = (int8_t)*code;
    return code + 1;
  }
  if (*code > END) {
    return callNative(code + 1, *code - END - 1, value);
  }
  if (!value) return skip(code);
  switch ((opcode_t)*code++) {
    case INT8:
      *value = (int8_t)*code;
      return code + 1;
    case INT16:
      *value = (int16_t)((*code<<8) | (*(code + 1)));
      return code + 2;
    case INT32:
      *value = (int32_t)(
         (*code<<24) |
        (*(code + 1) << 16) |
        (*(code + 2) << 8) |
         *(code + 3));
      return code + 4;
    case FOREVER: {
      while (true) {
        eval(code, value);
      }
      return code;
    }
    case WHILE: {
      int32_t cond;
      uint8_t *start = code;
      uint8_t *block = eval(start, &cond);
      if (!cond) {
        *value = 0;
        return skip(block);
      }
      do {
        code = eval(block, value);
        eval(start, &cond);
      } while (cond);
      return code;
    }
    case IF: {
      int32_t cond;
      while (true) {
        code = eval(code, &cond);
        if (cond) {
          code = eval(code, value);
          break;
        }
        if (*code == ELIF) {
          code++;
          continue;
        }
        if (*code == ELSE) {
          return eval(code + 1, value);
        }
      }
      while (*code == ELIF) {
        code = skip(skip(++code));
      }
      if (*code == ELSE) {
        code = skip(++code);
      }
    }
    case FOR: {
      int32_t i, end, incr, slot;
      uint8_t *start = eval(eval(eval(eval(code, &i), &end), &incr), &slot);
      slot += offset;
      if (incr == 0) incr = i < end ? 1 : -1;
      if (incr > 0 ? i > end : i < end) {
        *value = 0;
        return skip(code);
      }
      for (;incr > 0 ? i <= end : i >= end; i += incr) {
        stack[slot] = i;
        code = eval(start, value);
      }
      return code;
    }
    case SET: {
      int32_t slot;
      code = eval(eval(code, &slot), value);
      slot += offset;
      stack[slot] = *value;
      if (slot > highest) highest = slot;
      return code;
    }
    case GET: {
      int32_t slot;
      code = eval(code, &slot);
      *value = stack[offset + slot];
      return code;
    }
    case GSET: {
      int32_t slot;
      code = eval(eval(code, &slot), value);
      stack[slot] = *value;
      if (slot > highest) highest = slot;
      return code;
    }
    case GGET: {
      int32_t slot;
      code = eval(code, &slot);
      *value = stack[slot];
      return code;
    }
    case DEF: {
      int32_t slot;
      uint8_t *block = eval(code, &slot);
      code = skip(block);
      free(user[slot]);
      int len = code - block;
      user[slot] = malloc(len);
      memcpy(user[slot], block, len);
      return code;
    }
    case CALL: {
      int32_t slot;
      int start = highest + 1;
      int old = offset;
      code = eval(code, &slot);
      offset = start;
      highest = start;
      eval(user[slot], value);
      highest = start;
      offset = old;
      return code;
    }
    case CALL1: {
      int32_t slot;
      int start = highest + 1;
      int old = offset;
      code = eval(eval(code, &slot), stack + start);
      offset = start;
      highest = start + 1;
      eval(user[slot], value);
      highest = start;
      offset = old;
      return code;
    }
    case CALL2: {
      int32_t slot;
      int start = highest + 1;
      int old = offset;
      code = eval(eval(eval(code, &slot), stack + start), stack + start + 1);
      offset = start;
      highest = start + 1;
      eval(user[slot], value);
      highest = start;
      offset = old;
      return code;
    }
    case CALL3: {
      int32_t slot;
      int start = highest + 1;
      int old = offset;
      code = eval(eval(eval(eval(code, &slot), stack + start), stack + start + 1), stack + start + 2);
      offset = start;
      highest = start + 1;
      eval(user[slot], value);
      highest = start;
      offset = old;
      return code;
    }
    case CALL4: {
      int32_t slot;
      int start = highest + 1;
      int old = offset;
      code = eval(eval(eval(eval(eval(code, &slot), stack + start), stack + start + 1), stack + start + 2), stack + start + 3);
      offset = start;
      highest = start + 1;
      eval(user[slot], value);
      highest = start;
      offset = old;
      return code;
    }

    case RETURN: {
      code = eval(code, value);
      // TODO: early return
      return code;
    }
    case BREAK: {
      code = eval(code, value);
      // TODO: loop break
      return code;
    }
    case CONTINUE: {
      // TODO: loop continue
      return code;
    }

    case ADD: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a + b;
      return code;
    }
    case SUB: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a - b;
      return code;
    }
    case MUL: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a * b;
      return code;
    }
    case DIV: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a / b;
      return code;
    }
    case MOD: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a % b;
      return code;
    }
    case NEG: {
      int32_t a;
      code = eval(code, &a);
      *value = -a;
      return code;
    }
    case BAND: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a & b;
      return code;
    }
    case BOR: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a | b;
      return code;
    }
    case BXOR: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a ^ b;
      return code;
    }
    case BNOT: {
      int32_t a;
      code = eval(code, &a);
      *value = ~a;
      return code;
    }
    case LSHIFT: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a << b;
      return code;
    }
    case RSHIFT: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a >> b;
      return code;
    }
    case GT: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a > b;
      return code;
    }
    case GTE: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a >= b;
      return code;
    }
    case LT: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a < b;
      return code;
    }
    case LTE: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a <= b;
      return code;
    }
    case EQ: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a == b;
      return code;
    }
    case NEQ: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a != b;
      return code;
    }
    case AND: {
      code = eval(code, value);
      return eval(code, *value ? value : NULL);
    }
    case OR: {
      code = eval(code, value);
      return eval(code, *value ? NULL : value);
    }
    case XOR: {
      int32_t a, b;
      code = eval(eval(code, &a), &b);
      *value = a ? (b ? 0 : a) : (b ? b : 0);
      return code;
    }
    case NOT: {
      code = eval(code, value);
      *value = !*value;
      return code;
    }

    case DO: {
      if (*code == END) {
        *value = 0;
      }
      else while (*code != END) {
        code = eval(code, value);
      }
      return code + 1;
    }

    case ELIF: case ELSE: case END: return code;
  }
}


bool test(uint8_t *start, int len, int32_t expected) {
  uint8_t *code;
  int32_t value;
  code = eval(start, &value);
  printf("code-start=%d len=%d\n", code - start, len);
  if (!(value == expected && (code - start == expected))) return false;
  code = skip(start);
  return code - start == expected;
}

int main() {
  printf("Hello World\n");
  printf("%d slots left\n", 191-END);

  assert(test((uint8_t[]){10}, 1, 10));
  assert(test((uint8_t[]){-10}, 1, -10));
  assert(test((uint8_t[]){-10}, 1, -10));
  assert(test((uint8_t[]){INT8, 200}, 2, (int8_t)200));
  assert(test((uint8_t[]){INT16, 200, 100}, 3, (int16_t)((200<<8)|100)));
  assert(test((uint8_t[]){INT32, 200, 100, 50, 25}, 5, (int32_t)((200<<24)|(100<<16)|(50<<8)|25)));
  assert(test((uint8_t[]){ADD, 3, 2}, 3, 3 + 2));
  assert(test((uint8_t[]){SUB, 3, 2}, 3, 3 - 2));
  assert(test((uint8_t[]){MUL, 3, 4}, 3, 3 * 4));
  assert(test((uint8_t[]){DIV, 12, 3}, 3, 12 / 3));
  assert(test((uint8_t[]){MOD, 55, 10}, 3, 55 % 10));
  assert(test((uint8_t[]){NEG, 20}, 2, -20));
  assert(test((uint8_t[]){BAND, 0xf, 0x4}, 3, 0xf & 0x4));
  assert(test((uint8_t[]){BOR, 0xf, 0x4}, 3, 0xf | 0x4));
  assert(test((uint8_t[]){BXOR, 0x3, 0xf}, 3, 0x3 ^ 0xf));
  assert(test((uint8_t[]){BNOT, 0x3}, 2, ~0x3));
  assert(test((uint8_t[]){LSHIFT, 0x3, 0xf}, 3, 0x3 << 0xf));
  assert(test((uint8_t[]){RSHIFT, 0x3, 0xf}, 3, 0x3 >> 0xf));
  assert(test((uint8_t[]){LT, 1, 2}, 3, 1 < 2));
  assert(test((uint8_t[]){LT, 1, 1}, 3, 1 < 1));
  assert(test((uint8_t[]){LT, 2, 1}, 3, 2 < 1));
  assert(test((uint8_t[]){LTE, 1, 2}, 3, 1 <= 2));
  assert(test((uint8_t[]){LTE, 1, 1}, 3, 1 <= 1));
  assert(test((uint8_t[]){LTE, 2, 1}, 3, 2 <= 1));
  assert(test((uint8_t[]){GT, 1, 2}, 3, 1 > 2));
  assert(test((uint8_t[]){GT, 1, 1}, 3, 1 > 1));
  assert(test((uint8_t[]){GT, 2, 1}, 3, 2 > 1));
  assert(test((uint8_t[]){GTE, 1, 2}, 3, 1 >= 2));
  assert(test((uint8_t[]){GTE, 1, 1}, 3, 1 >= 1));
  assert(test((uint8_t[]){GTE, 2, 1}, 3, 2 >= 1));
  assert(test((uint8_t[]){EQ, 1, 2}, 3, 1 == 2));
  assert(test((uint8_t[]){EQ, 1, 1}, 3, 1 == 1));
  assert(test((uint8_t[]){EQ, 2, 1}, 3, 2 == 1));
  assert(test((uint8_t[]){NEQ, 1, 2}, 3, 1 != 2));
  assert(test((uint8_t[]){NEQ, 1, 1}, 3, 1 != 1));
  assert(test((uint8_t[]){NEQ, 2, 1}, 3, 2 != 1));
  assert(test((uint8_t[]){AND, 7, 0}, 3, 0));
  assert(test((uint8_t[]){AND, 7, 5}, 3, 5));
  assert(test((uint8_t[]){AND, 0, 5}, 3, 0));
  assert(test((uint8_t[]){AND, 0, 0}, 3, 0));
  assert(test((uint8_t[]){OR, 7, 0}, 3, 7));
  assert(test((uint8_t[]){OR, 7, 5}, 3, 7));
  assert(test((uint8_t[]){OR, 0, 5}, 3, 5));
  assert(test((uint8_t[]){OR, 0, 0}, 3, 0));
  assert(test((uint8_t[]){XOR, 7, 0}, 3, 7));
  assert(test((uint8_t[]){XOR, 7, 5}, 3, 0));
  assert(test((uint8_t[]){XOR, 0, 5}, 3, 5));
  assert(test((uint8_t[]){XOR, 0, 0}, 3, 0));
  assert(test((uint8_t[]){NOT, 0}, 2, 1));
  assert(test((uint8_t[]){NOT, 6}, 2, 0));
  assert(test((uint8_t[]){SET, 0, 42}, 3, 42));
  assert(test((uint8_t[]){GET, 0}, 2, 42));
  assert(test((uint8_t[]){FOR, 1, 10, 1, 1, DELAY, GET, 1}, 8, 10));
  assert(test((uint8_t[]){FOR, 20, 10, -1, 1, DELAY, GET, 1}, 8, 10));
  assert(test((uint8_t[]){WHILE, GET, 1, DELAY, SET, 1, SUB, GET, 1, 1}, 10, 0));
  return 0;
}
