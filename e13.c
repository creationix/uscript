#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

// stack based
typedef enum {
  // Numbers under 128 represent themselves
  INT32 = 128, INT64,
  // ADD, SUB, MUL, DIV, MOD, NEG,
  // LT, LTE, GT, GTE, EQ, NEQ,
  // AND, OR, XOR, NOT,
  IST, ISF, JMP, // jump - 0 is last
  POP,
  RADD, RSUB, RMUL, RDIV, RMOD, RNEG, // d/s - 0 is last
  INCR, DECR, // d/v
  CALL, END,
  DUMP,
} opcode;

const char* names[] = {
  "INT32", "INT64",
  "IST", "ISF", "JMP",
  "POP",
  "RADD", "RSUB", "RMUL", "RDIV", "RMOD", "RNEG",
  "INCR", "DECR",
  "CALL", "END",
  "DUMP",
  NULL
};

// Macro to make 16-bit constants as bytes
#define Int16(val) ((uint16_t)(val) & 0xff), ((uint16_t)(val) >> 8)
// Macro to make 32-bit constants as bytes
#define Int32(val) ((uint32_t)(val) & 0xff), \
                  (((uint32_t)(val) >> 8) & 0xff), \
                  (((uint32_t)(val) >> 16) & 0xff), \
                   ((uint32_t)(val) >> 24)

uint8_t code[] = {
  1,
  CALL, 0x11, Int16(84),
  DUMP,
  10,
  CALL, 0x11, Int16(78),
  DUMP,
  100,
  CALL, 0x11, Int16(72),
  DUMP,
  INT32, Int32(1000),
  CALL, 0x11, Int16(62),
  DUMP,
  INT32, Int32(10000),
  CALL, 0x11, Int16(52),
  DUMP,
  INT32, Int32(100000),
  CALL, 0x11, Int16(42),
  DUMP,
  INT32, Int32(1000000),
  CALL, 0x11, Int16(32),
  DUMP,
  INT32, Int32(10000000),
  CALL, 0x11, Int16(22),
  DUMP,
  INT32, Int32(100000000),
  CALL, 0x11, Int16(12),
  DUMP,
  INT32, Int32(1000000000),
  CALL, 0x11, Int16(2),
  DUMP,
  END,
  0,                        // sum = 0
  RADD, 0x10,               // sum += count
  DECR, 0x01,               // count -= 1
  IST, 0x0, Int16(-8),
  DUMP,
  END,
};

int64_t stack[100];
int64_t* last;
int64_t* first;

// slot is 4-bit number
// high bit set means go backwards from last
// low bit set means go forwards from first
// 0-7 registers/arguments
// 8-f stack last
#define slot(x) *(((x) & 8) ? (last - ((x) & 3)) : (first + ((x) & 3)))

void dump() {
  printf("%td-%td (", first - stack, last - stack);
  int64_t* i;
  for (i = first; i <= last; i++) {
    printf(" %"PRId64, *i);
  }
  printf(" )\n");
}

uint8_t* eval(uint8_t* pc) {
  while (true) {
    if (*pc < 128) {
      // printf("%d\n", *pc);
      *++last = *pc++;
      continue;
    }
    // printf("%td: %s\n", pc - code, names[*pc - 128]);
    switch ((opcode)*pc++) {
      case INT32:
        *++last = *(int32_t*)pc;
        pc += 4; continue;
      case INT64:
        *++last = *(int64_t*)pc;
        pc += 8; continue;
      case IST:
        if (slot(*pc)) { pc++; pc += *(int16_t*)pc + 2; }
        else pc += 3;
        continue;
      case ISF:
        if (!slot(*pc)) { pc++; pc += *(int16_t*)pc + 2; }
        else pc += 3;
        continue;
      case JMP:
        pc += *(int16_t*)pc + 2;
        continue;
      case POP: last--; continue;
      case RADD:
        slot(*pc >> 4) += slot(*pc & 0xf);
        pc++; continue;
      case RSUB:
        slot(*pc >> 4) -= slot(*pc & 0xf);
        pc++; continue;
      case RMUL:
        slot(*pc >> 4) *= slot(*pc & 0xf);
        pc++; continue;
      case RDIV:
        slot(*pc >> 4) /= slot(*pc & 0xf);
        pc++; continue;
      case RMOD:
        slot(*pc >> 4) %= slot(*pc & 0xf);
        pc++; continue;
      case RNEG:
        slot(*pc >> 4) = -slot(*pc >> 4);
        pc++; continue;
      case INCR:
        slot(*pc >> 4) += (*pc & 0xf);
        pc++; continue;
      case DECR:
        slot(*pc >> 4) -= (*pc & 0xf);
        pc++; continue;
      case CALL: {
        int64_t* old = first;
        first = last - (*pc >> 4) + 1;
        int8_t out = *pc++ & 0xf;
        eval(pc + *(int16_t*)pc + 2);
        pc += 2;
        // If there are extra slots, shift them out.
        if (last >= first + out) {
          last -= out;
          while (out--) *first++ = *++last;
          last = first - 1;
        }
        first = old;
        continue;
      }
      case END: return pc;
      case DUMP: dump(); continue;
    }
    assert(0);
  }
  return pc;
}

int main() {
  first = stack;
  last = stack - 1;
  eval(code);
  return 0;
}
