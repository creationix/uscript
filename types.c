#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <poll.h>
#include <unistd.h>

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

typedef enum type_e {
  Boolean,
  Integer,
  Pin, // Integer internally
  Mode, // Integer internally
  Address, // Integer internally
  ByteArray,
  WordArray,
} type_t;

typedef enum mode_e {
  Input,
  Output,
  InputPullup
} mode_t;

typedef enum opcode_e {
  // Values under 0x80 start variable length unsigned integer literals.
  // 0mxxxxxx mxxxxxxx*
  End = 0x80,
  L7, // Integer Integer -- Integer
  False, // -- Boolean
  True,  // -- Boolean
  Forward, // Boolean Integer --
  Back,    // Boolean Integer --
  While,   // Integer Integer --
  PrintI,  // Integer --
  Decr,// Integer -> Integer
  Incr,// Integer -> Integer
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
  SetPinMode,   // Pin Mode --
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

typedef struct compiler_s {
  type_t types[16];
  int top;
} compiler_t;

#define push(T) values[++top]
#define pop(T) values[top--]
#define top(T) values[top]
#define next(T) values[top + 1]
#define next2(T) values[top + 2]
#define prev(T) values[top - 1]
#define prev2(T) values[top - 2]

static bool step(thread_t* TT) {
  value_t values[16];
  int top = TT->top;
  uint8_t* pc = TT->pc;
  for(;;) {
  // printf("%p=%02x |", (void*)pc, *pc);
  // for (int i = 0; i <= top; i++) {
  //   printf(" %ld", values[i].integer);
  // }
  // printf("\n");

  if ((*pc & 0x80) == 0) {
    push(T).integer = (int32_t)(*pc++ & 0x7f);
    continue;
  }
  switch ((opcode_t)*pc++) {
    case End: return false;
    case L7: // Integer Integer -- Integer
      top--;
      top(T).integer = (top(T).integer << 7) | next(T).integer;
      continue;
    case False: // -- Boolean
      push(T).boolean = false;
      continue;
    case True: // -- Boolean
      push(T).boolean = true;
      continue;
    case Forward: // Integer Integer --
      if (prev(T).integer) {
        pc += top(T).integer;
      }
      top -= 2;
      continue;
    case Back: // Integer Integer --
      if (prev(T).integer) {
        pc -= top(T).integer;
      }
      top -= 2;
      continue;
    case PrintI: // Integer --
      printf("%ld\n", top(T).integer);
      top--;
      continue;
    case Incr: // Integer -> Integer
      top(T).integer++;
      continue;
    case Decr: // Integer -> Integer
      top(T).integer--;
      continue;
    case Add: // Integer Integer -- Integer
      top--;
      top(T).integer = top(T).integer + next(T).integer;
      continue;
    case Sub: // Integer Integer -- Integer
      top--;
      top(T).integer = top(T).integer - next(T).integer;
      continue;
    case Mul: // Integer Integer -- Integer
      top--;
      top(T).integer = top(T).integer * next(T).integer;
      continue;
    case Div: // Integer Integer -- Integer
      top--;
      top(T).integer = top(T).integer / next(T).integer;
      continue;
    case Mod: // Integer Integer -- Integer
      top--;
      top(T).integer = top(T).integer % next(T).integer;
      continue;
    case Neg: // Integer -- Integer
      top(T).integer = -top(T).integer;
      continue;
    case Lt: // Integer Integer -- Boolean
      top--;
      top(T).integer = top(T).integer < next(T).integer;
      continue;
    case Lte: // Integer Integer -- Boolean
      top--;
      top(T).boolean = top(T).integer <= next(T).integer;
      continue;
    case Gt: // Integer Integer -- Boolean
      top--;
      top(T).boolean = top(T).integer > next(T).integer;
      continue;
    case Gte: // Integer Integer -- Boolean
      top--;
      top(T).boolean = top(T).integer >= next(T).integer;
      continue;
    case Eq: // Integer Integer -- Boolean
      top--;
      top(T).boolean = top(T).integer == next(T).integer;
      continue;
    case Neq: // Integer Integer -- Boolean
      top--;
      top(T).boolean = top(T).integer != next(T).integer;
      continue;
    case And: // Boolean Boolean -- Boolean
      top--;
      top(T).boolean = top(T).boolean && next(T).boolean;
      continue;
    case Or: // Boolean Boolean -- Boolean
      top--;
      top(T).boolean = top(T).boolean || next(T).boolean;
      continue;
    case Xor: // Boolean Boolean -- Boolean
      top--;
      top(T).boolean = top(T).boolean != next(T).boolean;
      continue;
    case Not: // BOolean -- BOolean
      top(T).boolean = !top(T).boolean;
      continue;
    case Drop: // a --
      top--;
      continue;
    case Dup: // a -- a a
      next(T) = top(T);
      top++;
      continue;
    case Swap: { // a b -- b a
      value_t t = top(T);
      top(T) = prev(T);
      prev(T) = t;
      continue;
    }
    case Rot: { // a b c -- b c a
      value_t t = prev2(T);
      prev2(T) = prev(T);
      prev(T) = top(T);
      top(T) = t;
      continue;
    }
    case Nip:
      prev(T) = top(T);
      top--;
      continue;
    case NRot: { // a b c -- c a b
      value_t t = top(T);
      top(T) = prev(T);
      prev(T) = prev2(T);
      prev2(T) = t;
      continue;
    }
    case Tuck: // a b -- b a b
      next(T) = top(T);
      top(T) = prev(T);
      prev(T) = next(T);
      top++;
      continue;
    case Over: // a b -- a b a
      next(T) = prev(T);
      top++;
      continue;

    default:
      printf("TODO: Implement %d\n", *(pc - 1));
      return false;
  }
  }
  return false;
}

// Numbers up to 127
#define Uint7(n) (n) & 0x7f
// Numbers up to 16383
#define Uint14(n) (((n) >> 7) & 0x7f),\
                  (n) & 0x7f, L7
// Numbers up to 2097151
#define Uint22(n) (((n) >> 14) & 0x7f),\
                  (((n) >> 7) & 0x7f), L7,\
                  (n) & 0x7f, L7

// Numbers up to 268435455
#define Uint28(n) (((n) >> 21) & 0x7f),\
                  (((n) >> 14) & 0x7f), L7,\
                  (((n) >> 7) & 0x7f), L7,\
                  (n) & 0x7f, L7

// Numbers up to 4294967295
#define Uint35(n) (((n) >> 28) & 0xf),\
                  (((n) >> 21) & 0x7f), L7,\
                  (((n) >> 14) & 0x7f), L7,\
                  (((n) >> 7) & 0x7f), L7,\
                  (n) & 0x7f, L7

static struct pollfd fds[18];

int main() {

  fds[0].fd = 0;
  fds[0].events = POLLIN;

  thread_t* T = calloc(1, sizeof(thread_t));
  // Sum up all the integers from 1 to 1000000000
  uint8_t code[] = {
    Uint35(1000000000), 0,
    Over, Add, Swap,
    Decr,
    Tuck,
    7, Back,
    PrintI,
    End
  };
  T->pc = code;
  T->top = -1;
  while (step(T));

  // struct termios old_tio, new_tio;
	// tcgetattr(STDIN_FILENO, &old_tio);
	// new_tio = old_tio;
	// /* disable canonical mode (buffered i/o) and local echo */
	// new_tio.c_lflag &= (unsigned)(~ICANON & ~ECHO);
	// tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  //
  // printf("Polling\n");
  // while (poll(fds, 1, -1)) {
  //   uint8_t c;
  //   read(fds[0].fd, &c, 1);
  //   printf("%d\n", c);
  // }
  // printf("after\n");
  //
  // tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

  return 0;
}
