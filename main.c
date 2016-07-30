#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


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

uint8_t* callNative(uint8_t *code, int slot, int32_t *value) {
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

uint8_t* eval(uint8_t *code, int32_t *value) {
  if (*code < 64 || *code >= 192) {
    *value = (int8_t)*code;
    return code + 1;
  }
  if (*code > END) {
    return callNative(code + 1, *code - END - 1, value);
  }
  switch ((opcode_t)*code++) {
    case INT8: 
      *value = (int8_t)*code;
      return code + 1;
    case INT16:
      *value = (int16_t)(*code | ((*code + 1) << 8));
      return code + 2; 
    case INT32:
      *value = (int32_t)(*code | 
        ((*code + 1) << 8) |
        ((*code + 2) << 16) |
        ((*code + 3) << 24));
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
  }
}


int main() {
  printf("Hello World\n");
  printf("%d slots left\n", 191-END);

  uint8_t code[] = {
    DELAY, DIV, NEG, MUL, ADD, 1, 2, SUB, 10, 3, 5
  };
  uint32_t result;
  eval(code, &result);
  printf("result=%d\n", result);
  return 0;
}
