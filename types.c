#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

typedef struct byte_array_s {
  intptr_t* length;
  uint8_t data[];
} byte_array_t;

typedef struct word_array_s {
  intptr_t* length;
  uint32_t data[];
} word_array_t;

typedef union value_u {
  bool boolean;
  intptr_t integer;
  byte_array_t* byte_array;
  word_array_t* word_array;
} value_t;

// Abstract types in compiler:
//   Pin (Integer)
//   PinMode (Integer)
//   Address (Integer)

typedef enum opcode_e {
  // Values under 0x80 start variable length unsigned integer literals.
  // 0mxxxxxx mxxxxxxx*
  End = 0x80,
  False, // -- Boolean
  True,  // -- Boolean
  Add, // Integer Integer -- Integer
  Sub, // Integer Integer -- Integer
  Mul, // Integer Integer -- Integer
  Div, // Integer Integer -- Integer
  Mod, // Integer Integer -- Integer
  Neg, //         Integer -- Integer
  Lt,  // Integer Integer -- Boolean
  Lte, // Integer Integer -- Boolean
  Gt,  // Integer Integer -- Boolean
  Gte, // Integer Integer -- Boolean
  Eq,  // Integer Integer -- Boolean
  Neq, // Integer Integer -- Boolean
  And, // Boolean Boolean -- Boolean
  Or,  // Boolean Boolean -- Boolean
  Xor, // Boolean Boolean -- Boolean
  Not, //         Boolean -- Boolean
  Drop,// a --
  Dup, // a -- a a
  Swap,// a b -- b a
  Rot, // a b c -- b c a
  Nip, // a b  -- b
  NRot,// a b c -- c a b
  Tuck,// a b -- b a b
  Over,// a b -- a b a
  PinMode,      // Pin Mode --
  DigitalWrite, // Pin Boolean --
  DigitalRead,  // Pin -- Boolean
  Tone,         // Pin Integer Integer --
  Delay,        // Integer
  Begin,        // Pin Pin --
  WriteI,       // Integer --
  WriteB,       // ByteArray --
  WriteW,       // WordArray --
  Available,    // -- Integer
  Read,         // -- Integer
  RequestFrom,  // Address Integer Boolean --
  BeginTransmission, // Address --
  EndTransmission,   // Boolean --
  NewByteArray, // Integer -- ByteArray
  ByteRead,     // ByteArray Integer -- Integer
  ByteWrite,    // ByteArray Integer Integer --
  ByteLength,   // ByteArray -- Integer
  NewWordArray, // Integer -- WordArray
  WordRead,     // WordArray Integer -- Integer
  WordWrite,    // WordArray Integer Integer --
  WordLength,   // WordArray -- Integer
  NeopixelB,    // Pin ByteArray --
  NeopixelW,    // Pin WordArray --
} opcode_t;

typedef struct thread_s {
  uint8_t* pc;
  value_t values[16];
  uint8_t* calls[16];
  int top;
  int depth;
} thread_t;

#define push(T) T->values[++T->top]
#define pop(T) T->values[T->top--]
#define top(T) T->values[T->top]
#define next(T) T->values[T->top + 1]
#define next2(T) T->values[T->top + 2]
#define prev(T) T->values[T->top - 1]
#define prev2(T) T->values[T->top - 2]

bool step(thread_t* T) {
  printf("%p=%02x |", (void*)T->pc, *T->pc);
  for (int i = 0; i <= T->top; i++) {
    printf(" %ld", T->values[i].integer);
  }
  printf("\n");

  if (!(*T->pc & 0x80)) {
    int32_t num = *T->pc & 0x3f;
    if (*T->pc++ & 0x40) {
      do {
        num = (num << 7) | (*T->pc & 0x7f);
      } while (*T->pc++ & 0x80);
    }
    push(T).integer = (int32_t)num;
    return true;
  }
  switch ((opcode_t)*T->pc++) {
    case End: return false;
    case False:
      push(T).boolean = false;
      return true;
    case True:
      push(T).boolean = true;
      return true;
    case Add: // Integer Integer -- Integer
      T->top--;
      top(T).integer = top(T).integer + next(T).integer;
      return true;
    case Sub: // Integer Integer -- Integer
      T->top--;
      top(T).integer = top(T).integer - next(T).integer;
      return true;
    case Mul: // Integer Integer -- Integer
      T->top--;
      top(T).integer = top(T).integer * next(T).integer;
      return true;
    case Div: // Integer Integer -- Integer
      T->top--;
      top(T).integer = top(T).integer / next(T).integer;
      return true;
    case Mod: // Integer Integer -- Integer
      T->top--;
      top(T).integer = top(T).integer % next(T).integer;
      return true;
    case Neg: // Integer -- Integer
      top(T).integer = -top(T).integer;
      return true;
    case Lt: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer < next(T).integer;
      return true;
    case Lte: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer <= next(T).integer;
      return true;
    case Gt: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer > next(T).integer;
      return true;
    case Gte: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer >= next(T).integer;
      return true;
    case Eq: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer == next(T).integer;
      return true;
    case Neq: // Integer Integer -- Boolean
      T->top--;
      top(T).boolean = top(T).integer != next(T).integer;
      return true;
    case And: // Boolean Boolean -- Boolean
      T->top--;
      top(T).boolean = top(T).boolean && next(T).boolean;
      return true;
    case Or: // Boolean Boolean -- Boolean
      T->top--;
      top(T).boolean = top(T).boolean || next(T).boolean;
      return true;
    case Xor: // Boolean Boolean -- Boolean
      T->top--;
      top(T).boolean = top(T).boolean != next(T).boolean;
      return true;
    case Not: // BOolean -- BOolean
      top(T).boolean = !top(T).boolean;
      return true;
    case Drop: // a --
      T->top--;
      return true;
    case Dup: // a -- a a
      next(T) = top(T);
      T->top++;
      return true;
    case Swap: { // a b -- b a
      value_t t = top(T);
      top(T) = prev(T);
      prev(T) = t;
      return true;
    }
    case Rot: { // a b c -- b c a
      value_t t = prev2(T);
      prev2(T) = prev(T);
      prev(T) = top(T);
      top(T) = t;
      return true;
    }
    case Nip:
      prev(T) = top(T);
      T->top--;
      return true;
    case NRot: { // a b c -- c a b
      value_t t = top(T);
      top(T) = prev(T);
      prev(T) = prev2(T);
      prev2(T) = t;
      return true;
    }
    case Tuck:
      next(T) = top(T);
      top(T) = prev(T);
      prev(T) = next(T);
      T->top++;
      return true;
    case Over:
      next(T) = prev(T);
      T->top++;
      return true;

    default:
      printf("TODO: Implement %d\n", *(T->pc - 1));
      return false;
  }
}

// Numbers up to 64
#define Uint6(n) (n) & 0x3f
// Numbers up to 8191
#define Uint13(n) (0x40|(((n) >> 7) & 0x3f)),\
                  (n) & 0x7f
// Numbers up to 1048576
#define Uint20(n) (0x40|(((n) >> 14) & 0x3f)),\
                  (0x80|(((n) >> 7) & 0x7f)),\
                  (n) & 0x7f
// Numbers up to 134217728
#define Uint27(n) (0x40|(((n) >> 21) & 0x3f)),\
                  (0x80|(((n) >> 14) & 0x7f)),\
                  (0x80|(((n) >> 7) & 0x7f)),\
                  (n) & 0x7f
// Numbers up to 17179869184
#define Uint34(n) (0x40|(((n) >> 28) & 0x3f)),\
                  (0x80|(((n) >> 21) & 0x7f)),\
                  (0x80|(((n) >> 14) & 0x7f)),\
                  (0x80|(((n) >> 7) & 0x7f)),\
                  (n) & 0x7f

int main() {
  thread_t* T = calloc(1, sizeof(thread_t));
  uint8_t code[] = {
    Uint13(1000), Uint34(-1), 1, Neg,
    1, 2, Add,
    4, Tuck, Dup, Sub,
    End
  };
  T->pc = code;
  T->top = -1;
  while (step(T));
  return 0;
}
