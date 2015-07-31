#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#define PLATFORM_INCLUDES
#include "platform.c"
#undef PLATFORM_INCLUDES

#define number uint64_t

// Max depth for data stack
#define DMAX 16
static number dstack[DMAX];
static number* d;
// Max depth for local/return stack
#define RMAX 16
static number rstack[RMAX];
static number* r;

unsigned char *pc, *end;

enum opcodes {
  // Jumps/Conditionals
  // All consume the top value and use it as the jump distance in bytes.
  // Forward Jumps - always consume conditional.
  // 'Skip' always jumps, 'If' jumps if non-zero, 'unless' jumps if zero.
  OP_SKIP = 128, OP_IF, OP_UNLESS,
  // Backward Jumps - never consume conditional.
  // 'Repeat' always jumps, 'If' jumps if non-zero, 'until' jumps is zero.
  OP_REPEAT, OP_WHILE, OP_UNTIL,
  // Stack Operations
  OP_OVER, OP_DUP,
  OP_PUT, OP_GET, OP_SAVE,
  OP_SWAP, OP_POP,
  // Binary Operations, top is right, next is left.
  OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
  OP_AND, OP_OR, OP_XOR,
  // Unary Operations
  OP_INCR, OP_DECR, OP_NEG, OP_NOT,

  // Platform defined enums
  #define PLATFORM_OPCODES
  #include "platform.c"
  #undef PLATFORM_OPCODES
};

const char* opnames =
  "SKIP\0IF\0UNLESS\0"
  "REPEAT\0WHILE\0UNTIL\0"
  "OVER\0DUP\0"
  "PUT\0GET\0SAVE\0"
  "SWAP\0POP\0"
  "ADD\0SUB\0MUL\0DIV\0MOD\0"
  "AND\0OR\0XOR\0"
  "INCR\0DECR\0NEG\0NOT\0"
  #define PLATFORM_OPNAMES
  #include "platform.c"
  #undef PLATFORM_OPNAMES
  "\0";

static void dump(unsigned char* start) {
  printf("%04lu:", pc - start);
  number* i;
  for (i = dstack; i <= d; i++) printf(" %"PRId64, *i);
  printf(" -");
  for (i = r; i >= rstack; i--) printf(" %"PRId64, *i);
  printf("\n");
}

const char* eval() {
  unsigned char* start = pc;
  while (pc < end) {
    dump(start);
    if (*pc < 0x80) {
      number num = *pc & 0x3f;
      if (*pc++ & 0x40) {
        do {
          num = (num << 7) | (*pc & 0x7f);
        } while (*pc++ & 0x80);
      }
      *++d = num;
    }
    else switch ((enum opcodes)*pc++) {
      case OP_SKIP: pc += *d--; break;
      case OP_IF: {
        number jump = *d--;
        if (*d--) pc += jump;
        break;
      }
      case OP_UNLESS: {
        number jump = *d--;
        if (!(*d--)) pc += jump;
        break;
      }
      case OP_REPEAT: pc -= *d--; break;
      case OP_WHILE: {
        number jump = *d--;
        if (*d) pc -= jump;
        break;
      }
      case OP_UNTIL: {
        number jump = *d--;
        if (!*d) pc -= jump;
        break;
      }

      case OP_OVER: ++d; *d = *(d - 2); break;
      case OP_DUP:  ++d; *d = *(d - 1); break;
      case OP_PUT:  *++r = *d--; break;
      case OP_GET:  *++d = *r--; break;
      case OP_SAVE: *++r = *d; break;
      case OP_SWAP: {
        number temp = *d;
        *d = *(d - 1);
        *(d - 1) = temp;
        break;
      }
      case OP_POP: --d; break;

      case OP_ADD: --d; *d += *(d + 1); break;
      case OP_SUB: --d; *d -= *(d + 1); break;
      case OP_MUL: --d; *d *= *(d + 1); break;
      case OP_DIV: --d; *d /= *(d + 1); break;
      case OP_MOD: --d; *d %= *(d + 1); break;

      case OP_AND: if   (*--d)  *d = *(d + 1) ? *(d + 1) : 0; break;
      case OP_OR:  if (!(*--d)) *d = *(d + 1) ? *(d + 1) : 0; break;
      case OP_XOR:
        --d;
        *d = *d && !*(d + 1) ? *d :
             *(d + 1) && !*d ? *(d + 1) :
             0;
        break;

      case OP_INCR: ++*d; break;
      case OP_DECR: --*d; break;
      case OP_NEG: *d = -*d; break;
      case OP_NOT: *d = !*d; break;

      #define PLATFORM_CASES
      #include "platform.c"
      #undef PLATFORM_CASES
    }
  }
  return NULL;
}

// unsigned char code[] = {
//   0x2a, // 42
//   0x3a, // 58
//   0x40, 0x64, // 100
//   OP_ADD,
//   OP_ADD,
//   0x32, // 50
//   OP_SUB,
//   4, OP_WHILE, // Loop while not zero
//   10,
// };
// int code_len = 11;

unsigned char code[] = {
  0x3f, // 63
  0x3f, OP_OVER, OP_SUB,
  OP_PRINT,
  OP_DECR,
  7, OP_WHILE,
};
int code_len = 8;

// unsigned char code[] = {
//   0, // 0
//   0x43, 0xdc, 0xeb, 0x94, 0x00,  // 1000000000
//   OP_SAVE, // Save a copy of count
//   OP_ADD, // Add count into sum
//   OP_GET, // Restore count
//   OP_DECR, // Decrement count
//   6, OP_WHILE, // loop while count is non-zero
//   OP_POP, // pop the 0 count leaving the sum
// };
// int code_len = 13;

int main() {
  d = dstack - 1;
  r = rstack - 1;

  pc = code;
  end = code + code_len;
  const char* err = eval();

  dump(code);

  if (err) {
    printf("ERROR: %s\n", err);
    return -1;
  }
  return 0;
}
