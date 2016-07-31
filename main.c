#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
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
  FOR,     // (start, stop, incr, slot, block)
  IF,      // (cond, block)
  ELIF,    // (cond, block)
  ELSE,    // (block)

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
}

uint8_t* skip(uint8_t *code) {
  if (*code < 64 || *code >= 192) return code + 1;
  if (*code > END) {
    return callNative(code + 1, *code - END - 1, NULL);
  }
  switch ((opcode_t)*code++) {
    case INT8: return code + 1;
    case INT16: return code + 2;
    case INT32: return code + 4;
    case ADD: case SUB: case MUL: case DIV: case MOD:
    case BAND: case BOR: case BXOR: case LSHIFT: case RSHIFT:
    case GT: case GTE: case LT: case LTE: case EQ: case NEQ:
    case AND: case OR: case XOR:
      return skip(skip(code));
    case NEG: case NOT: case BNOT:
      return skip(code);
  }
}

uint8_t* eval(uint8_t *code, int32_t *value) {
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
  }
}


bool test(uint8_t *code, int32_t expected) {
  int32_t value;
  eval(code, &value);
  return value == expected;
}

int main() {
  printf("Hello World\n");
  printf("%d slots left\n", 191-END);

  assert(test((uint8_t[]){10}, 10));
  assert(test((uint8_t[]){-10}, -10));
  assert(test((uint8_t[]){-10}, -10));
  assert(test((uint8_t[]){INT8, 200}, (int8_t)200));
  assert(test((uint8_t[]){INT16, 200, 100}, (int16_t)((200<<8)|100)));
  assert(test((uint8_t[]){INT32, 200, 100, 50, 25}, (int32_t)((200<<24)|(100<<16)|(50<<8)|25)));
  assert(test((uint8_t[]){ADD, 3, 2}, 3 + 2));
  assert(test((uint8_t[]){SUB, 3, 2}, 3 - 2));
  assert(test((uint8_t[]){MUL, 3, 4}, 3 * 4));
  assert(test((uint8_t[]){DIV, 12, 3}, 12 / 3));
  assert(test((uint8_t[]){MOD, 55, 10}, 55 % 10));
  assert(test((uint8_t[]){NEG, 20}, -20));
  assert(test((uint8_t[]){BAND, 0xf, 0x4}, 0xf & 0x4));
  assert(test((uint8_t[]){BOR, 0xf, 0x4}, 0xf | 0x4));
  assert(test((uint8_t[]){BXOR, 0x3, 0xf}, 0x3 ^ 0xf));
  assert(test((uint8_t[]){BNOT, 0x3}, ~0x3));
  assert(test((uint8_t[]){LSHIFT, 0x3, 0xf}, 0x3 << 0xf));
  assert(test((uint8_t[]){RSHIFT, 0x3, 0xf}, 0x3 >> 0xf));
  assert(test((uint8_t[]){LT, 1, 2}, 1 < 2));
  assert(test((uint8_t[]){LT, 1, 1}, 1 < 1));
  assert(test((uint8_t[]){LT, 2, 1}, 2 < 1));
  assert(test((uint8_t[]){LTE, 1, 2}, 1 <= 2));
  assert(test((uint8_t[]){LTE, 1, 1}, 1 <= 1));
  assert(test((uint8_t[]){LTE, 2, 1}, 2 <= 1));
  assert(test((uint8_t[]){GT, 1, 2}, 1 > 2));
  assert(test((uint8_t[]){GT, 1, 1}, 1 > 1));
  assert(test((uint8_t[]){GT, 2, 1}, 2 > 1));
  assert(test((uint8_t[]){GTE, 1, 2}, 1 >= 2));
  assert(test((uint8_t[]){GTE, 1, 1}, 1 >= 1));
  assert(test((uint8_t[]){GTE, 2, 1}, 2 >= 1));
  assert(test((uint8_t[]){EQ, 1, 2}, 1 == 2));
  assert(test((uint8_t[]){EQ, 1, 1}, 1 == 1));
  assert(test((uint8_t[]){EQ, 2, 1}, 2 == 1));
  assert(test((uint8_t[]){NEQ, 1, 2}, 1 != 2));
  assert(test((uint8_t[]){NEQ, 1, 1}, 1 != 1));
  assert(test((uint8_t[]){NEQ, 2, 1}, 2 != 1));
  assert(test((uint8_t[]){AND, 7, 0}, 0));
  assert(test((uint8_t[]){AND, 7, 5}, 5));
  assert(test((uint8_t[]){AND, 0, 5}, 0));
  assert(test((uint8_t[]){AND, 0, 0}, 0));
  assert(test((uint8_t[]){OR, 7, 0}, 7));
  assert(test((uint8_t[]){OR, 7, 5}, 7));
  assert(test((uint8_t[]){OR, 0, 5}, 5));
  assert(test((uint8_t[]){OR, 0, 0}, 0));
  assert(test((uint8_t[]){XOR, 7, 0}, 7));
  assert(test((uint8_t[]){XOR, 7, 5}, 0));
  assert(test((uint8_t[]){XOR, 0, 5}, 5));
  assert(test((uint8_t[]){XOR, 0, 0}, 0));
  assert(test((uint8_t[]){NOT, 0}, 1));
  assert(test((uint8_t[]){NOT, 6}, 0));

//  uint8_t code[] = {
//    DELAY, DIV, NEG, MUL, ADD, 1, 2, SUB, 10, 3, 5
//  };
//  uint32_t result;
//  eval(code, &result);
//  printf("result=%d\n", result);
  return 0;
}
