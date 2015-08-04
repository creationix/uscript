#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

enum opcode {
  // Literal uint7_t is any code below 128
  CALL = 128, // A/B ADDRESS - Call code at address passing in slots A-B.
  TCALL,      // A/B ADDRESS - Tail call - call and return in one.
  RETURN,     // A/B - Return values from slots A-B inclusive
  INT16,      // C XX XX : Store int16_t at slot C
  INT32,      // C XX XX XX XX : Store int32_t at slot C
  INT64,      // C XX XX XX XX XX XX XX XX : Store int64_t at slot C

  // Register based arithmetic
  // Reads slots A and B and stores result in A
  COPY,       // A/B : A = B
  ADD,        // A/B : A += B
  SUB,        // A/B : A -= B
  ISUB,       // A/B : A = B - A
  MUL,        // A/B : A *= B
  DIV,        // A/B : A /= B
  IDIV,       // A/B : A = B / A
  MOD,        // A/B : A %= B
  IMOD,       // A/B : A = B % A
  NEG,        // A/B : A = -B

  AND,        // a && b
  OR,         // a || b
  XOR,        // a xor b
  NOT,        // !a

  // Stack manipulation
  DUP, OVER, SWAP, POP,




  DECR,
  INCR, NEG,

  LT, GT, LTE, GTE, EQ, NEQ,

  WHILE, UNTIL, JUMP,
  IST, ISF, ISTC, ISFC,

  GROW, DUP, OVER, COPY, POP,

  PM, DW, DR,
};

struct code {
  enum opcode op;
  union {
    struct {
      unsigned int A : 4;
      unsigned int B : 4;
    };
    int8_t C;
  };
};
// uint8_t code[] = {
//   0x00,                          // *++d = 0
//   INT32, 0x00, 0xca, 0x9a, 0x3b, // *++d = 1000000000
//   // top:
//   ADD,   0x10,                   // *(d - 1) += *(d - 0)
//   DECR,                          // *(d)--
//   WHILE, -5,                     // if *d goto top
//   POP,                           // --d
// };
// int len = 12;

uint8_t code[] = {
  PM, 0x51,  // PinMode(5, OUTPUT)
  PM, 0xC1,  // PinMode(12, OUTOUT)
  PM, 0xD1,  // PinMode(13, OUTOUT)
  PM, 0xE1,  // PinMode(14, OUTOUT)
  PM, 0x22,  // PinMode(2, INPUT_PULLUP)
  PM, 0x42,  // PinMode(4, INPUT_PULLUP)
  GROW, 5,   // Move top 5 spaces up in stack, fill with zero

  // Read pin 2 4, and store values and compliments in slots 1-4.
  DR, 0x42,
  DR, 0x34,
  NOT, 0x24,
  NOT, 0x13,
  COPY, 0x04,
  AND, 0x03,
  DW, 0x50,
  COPY, 0x02,
  AND, 0x03,
  DW, 0xC0,
  COPY, 0x04,
  AND, 0x01,
  DW, 0xD0,
  COPY, 0x02,
  AND, 0x01,
  DW, 0xE0,

  // Start over
  JUMP, -34,

};
int len = 48;

int64_t stack[16], *d;
uint8_t *pc, *end;

void setup() {
  d = stack - 1;
  pc = code;
  end = pc + len;
}

void loop() {

}
int main() {
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

    case NEG:
      *(d - (*pc >> 4)) = -(*(d - (*pc & 0xf)));
      ++pc; break;

    case NOT:
      *(d - (*pc >> 4)) = !(*(d - (*pc & 0xf)));
      ++pc; break;

    case AND:
      *(d - (*pc >> 4)) = !(*(d - (*pc & 0xf)));
      ++pc; break;

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
