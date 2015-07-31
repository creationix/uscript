#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

enum opcodes {
  INT8 = 128, INT16, INT32, INT64,
  ADD, SUB, MUL, DIV, MOD,
  ADDI, SUBI, MULI, DIVI, MODI,
  DECR, INCR,

  WHILE, UNTIL, JUMP,
  IST, ISF, ISTC, ISFC,
  LT, GT, LTE, GTE, EQ, NEQ,
  LTI, GTI, LTEI, GTEI, EQI, NEQI,

  DUP, OVER, COPY, POP,
};

uint8_t code[] = {
  0x00,                          // *++d = 0
  INT32, 0x00, 0xca, 0x9a, 0x3b, // *++d = 1000000000
  // top:
  ADD,   0x10,                   // *(d - 1) += *(d - 0)
  DECR,                          // *(d)--
  WHILE, -5,                     // if *d goto top
  POP,                           // --d
};
int len = 12;

int main() {
  int64_t stack[16];
  int64_t* d = stack - 1;
  uint8_t* pc = code;
  uint8_t* end = pc + len;
  while (pc < end) {
    // printf("%tu:", pc - code);
    // int64_t* i;
    // for (i = stack; i <= d; i++) {
    //   printf(" %"PRId64, *i);
    // }
    // printf("\n");

    if (*pc < 0x80) *++d = *pc++;
    else switch ((enum opcodes)*pc++) {
    case INT8:
      *++d = *((int8_t*)pc);
      pc += 1; break;
    case INT16:
      *++d = *((int16_t*)pc);
      pc += 2; break;
    case INT32:
      *++d = *((int32_t*)pc);
      pc += 4; break;
    case INT64:
      *++d = *((int64_t*)pc);
      pc += 8; break;
    case ADD:
      *(d - (*pc >> 4)) += *(d - (*pc & 0xf));
      ++pc; break;
    case SUB:
      *(d - (*pc >> 4)) -= *(d - (*pc & 0xf));
      ++pc; break;
    case MUL:
      *(d - (*pc >> 4)) *= *(d - (*pc & 0xf));
      ++pc; break;
    case DIV:
      *(d - (*pc >> 4)) /= *(d - (*pc & 0xf));
      ++pc; break;
    case MOD:
      *(d - (*pc >> 4)) %= *(d - (*pc & 0xf));
      ++pc; break;
    case ADDI:
      *(d - (*pc >> 4)) += (*pc & 0xf);
      ++pc; break;
    case SUBI:
      *(d - (*pc >> 4)) -= (*pc & 0xf);
      ++pc; break;
    case MULI:
      *(d - (*pc >> 4)) *= (*pc & 0xf);
      ++pc; break;
    case DIVI:
      *(d - (*pc >> 4)) /= (*pc & 0xf);
      ++pc; break;
    case MODI:
      *(d - (*pc >> 4)) %= (*pc & 0xf);
      ++pc; break;
    case DECR: *d -= 1; break;
    case INCR: *d += 1; break;
    case WHILE:
      if (*d) pc += (int8_t)*pc;
      pc += 1; break;
    case UNTIL:
      if (!(*d)) pc += (int8_t)*pc;
      pc += 1; break;
    case JUMP: pc += (int8_t)*pc + 1; break;
    case IST:
      if (*(d - *pc)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case ISF:
      if (!(*(d - *pc))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case ISTC: {
      int64_t num  = (*(d - (*pc & 0xf)));
      if (num) {
        *(d - (*pc >> 4)) = num;
        pc += (int8_t)*(pc + 1);
      }
      pc += 2; break;
    }
    case ISFC: {
      int64_t num  = (*(d - (*pc & 0xf)));
      if (!num) {
        *(d - (*pc >> 4)) = num;
        pc += (int8_t)*(pc + 1);
      }
      pc += 2; break;
    }
    case LT:
      if (*(d - (*pc >> 4)) < *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case GT:
      if (*(d - (*pc >> 4)) > *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case LTE:
      if (*(d - (*pc >> 4)) <= *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case GTE:
      if (*(d - (*pc >> 4)) >= *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case EQ:
      if (*(d - (*pc >> 4)) == *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case NEQ:
      if (*(d - (*pc >> 4)) != *(d - (*pc & 0xf))) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case LTI:
      if (*(d - (*pc >> 4)) < (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case GTI:
      if (*(d - (*pc >> 4)) > (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case LTEI:
      if (*(d - (*pc >> 4)) <= (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case GTEI:
      if (*(d - (*pc >> 4)) >= (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case EQI:
      if (*(d - (*pc >> 4)) == (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case NEQI:
      if (*(d - (*pc >> 4)) != (*pc & 0xf)) pc += (int8_t)*(pc + 1);
      pc += 2; break;
    case DUP: ++d; *d = *(d - 1); break;
    case OVER: ++d; *d = *(d - 2); break;
    case COPY: ++d; *d = *(d - *pc++); break;
    case POP: --d; break;
    }
  }
  printf("%tu:", pc - code);
  int64_t* i;
  for (i = stack; i <= d; i++) {
    printf(" %"PRId64, *i);
  }
  printf("\n");
}
