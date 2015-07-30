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
  OP_JUMP = 128, OP_NJUMP,
  OP_JZ, OP_NJZ,
  OP_ADD, OP_SUB,
  OP_NEG, OP_NOT,
  OP_OVER, OP_DUP,
  OP_PUT, OP_GET,
  OP_SWAP, OP_POP,
};

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
  // '01000011 11011100 11101011 10010100 00000000'
  0x43, 0xdc, 0xeb, 0x94, 0x00,  // 1000000000
  0x0, // 0
  OP_OVER, // copy count
  OP_ADD,
  OP_PUT,
  1, OP_SUB,
  OP_GET,
  OP_OVER, OP_NOT, 10, OP_NJZ, // loop while count left
  OP_SWAP, OP_POP,
};
int code_len = 18;

int main() {
  number dstack[DMAX];
  number* d = dstack - 1;
  number rstack[RMAX];
  number* r = rstack - 1;

  unsigned char* pc = code;
  unsigned char* end = pc + code_len;
  number z = 0;
  while (pc < end) {
    if (!(z++ % 100000000)) {
      number* i;
      // printf("\npc: %ld\nD:", pc - code);
      for (i = dstack; i <= d; i++) {
        printf(" %"PRId64, *i);
      }
      printf("\n");

    }
    // printf("\nR:");
    // for (i = rstack; i <= r; i++) {
    //   printf(" %"PRId64, *i);
    // }
    // printf("\n");
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
      case OP_JUMP:
        pc += *d--;
        break;
      case OP_NJUMP:
        pc -= *d--;
        break;
      case OP_JZ: {
        number jump = *d--;
        if (!(*d--)) pc += jump;
        break;
      }
      case OP_NJZ: {
        number jump = *d--;
        if (!(*d--)) pc -= jump;
        break;
      }
      case OP_ADD:
        --d;
        *d += *(d + 1);
        break;
      case OP_SUB:
        --d;
        *d -= *(d + 1);
        break;
      case OP_NEG:
        *d = -*d;
        break;
      case OP_NOT:
        *d = *d ? 0 : 1;
        break;
      case OP_DUP:
        ++d;
        *d = *(d - 1);
        break;
      case OP_OVER:
        ++d;
        *d = *(d - 2);
        break;
      case OP_SWAP: {
        number temp = *d;
        *d = *(d - 1);
        *(d - 1) = temp;
        break;
      }
      case OP_POP: {
        --d;
        break;
      }
      case OP_PUT:
        *++r = *d--;
        break;
      case OP_GET:
        *++d = *r--;
        break;
    }
  }

  number* i;
  printf("\npc: %ld\nD:", pc - code);
  for (i = dstack; i <= d; i++) {
    printf(" %"PRId64, *i);
  }
  printf("\nR:");
  for (i = rstack; i <= r; i++) {
    printf(" %"PRId64, *i);
  }
  printf("\n");


  return 0;
}
