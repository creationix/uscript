#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// 0mxxxxxx mxxxxxxx - unsigned integer literal (variable length)
//
// JUMP - jump to address
// JZ - jump if top is zero

#define DMAX 16
#define RMAX 16

#define number uint64_t
enum opcodes {
  OP_JF = 128, OP_JB,
  OP_IF_JF, OP_IF_JB,
  OP_UNLESS_JF, OP_UNLESS_JB,
  OP_WHILE, OP_UNTIL,
  OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
  OP_INCR, OP_DECR,
  OP_NEG, OP_NOT,
  OP_OVER, OP_DUP,
  OP_PUT, OP_GET, OP_SAVE,
  OP_SWAP, OP_POP,
};

#define FUNC(name, body) \
const char* name(number** dp, number** rp) { \
  number* d = *dp; number* r = *rp; const char* res = NULL; \
  body \
  *dp = d; *rp = r; return res; \
}

static FUNC(print, {
  printf("%"PRId64"\n", *--d);
})

typedef const char* (*userfunc)(number** dp, number** rp);

userfunc functions[] = {
  print,
};

const char* eval(unsigned char* pc, unsigned char* end, number** dp, number** rp) {
  number* d = *dp; number* r = *rp;
  while (pc < end) {
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
      case OP_JF: pc += *d--; break;
      case OP_JB: pc -= *d--; break;
      case OP_IF_JF: {
        number jump = *d--;
        if (*d--) pc += jump;
        break;
      }
      case OP_IF_JB: {
        number jump = *d--;
        if (*d--) pc -= jump;
        break;
      }
      case OP_UNLESS_JF: {
        number jump = *d--;
        if (!(*d--)) pc += jump;
        break;
      }
      case OP_UNLESS_JB: {
        number jump = *d--;
        if (!(*d--)) pc -= jump;
        break;
      }
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
      case OP_ADD: --d; *d += *(d + 1); break;
      case OP_SUB: --d; *d -= *(d + 1); break;
      case OP_MUL: --d; *d *= *(d + 1); break;
      case OP_DIV: --d; *d /= *(d + 1); break;
      case OP_MOD: --d; *d %= *(d + 1); break;
      case OP_INCR: ++*d; break;
      case OP_DECR: --*d; break;
      case OP_NEG: *d = -*d; break;
      case OP_NOT: *d = !*d; break;

      case OP_DUP:  ++d; *d = *(d - 1); break;
      case OP_OVER: ++d; *d = *(d - 2); break;
      case OP_SWAP: {
        number temp = *d;
        *d = *(d - 1);
        *(d - 1) = temp;
        break;
      }
      case OP_POP: --d; break;
      case OP_PUT: *++r = *d--; break;
      case OP_SAVE: *++r = *d; break;
      case OP_GET: *++d = *r--; break;
    }
  }
  *dp = d; *rp = r;
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
//   OP_DUP, OP_NOT, 6, OP_NJZ, // Loop while not zero
//   10,
// };
// int code_len = 13;

unsigned char code[] = {
  0, // 0
  0x43, 0xdc, 0xeb, 0x94, 0x00,  // 1000000000
  OP_SAVE, // Save a copy of count
  OP_ADD, // Add count into sum
  OP_GET, // Restore count
  OP_DECR, // Decrement count
  6, OP_WHILE, // loop while count is non-zero
  OP_POP, // pop the 0 count leaving the sum
};
int code_len = 13;

int main() {
  number dstack[DMAX];
  number* d = dstack - 1;
  number rstack[RMAX];
  number* r = rstack - 1;

  const char* err = eval(code, code + code_len, &d, &r);

  number* i;
  printf("D:");
  for (i = dstack; i <= d; i++) {
    printf(" %"PRId64, *i);
  }
  printf("\nR:");
  for (i = rstack; i <= r; i++) {
    printf(" %"PRId64, *i);
  }
  printf("\n");

  if (err) {
    printf("ERROR: %s\n", err);
    return -1;
  }
  return 0;
}
