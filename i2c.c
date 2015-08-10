#include "stdint.h"
#include "stdio.h"

typedef enum {
  // stack operations
  DROP = 128, // n ->
  DUP, // n -> n n
  DUP2, // a b -> a b a b
  OVER, // a b -> a b a
  SWAP, // a b -> b a
  ROT,  // a b c -> b c a

  ADD, SUB, MUL, DIV, MOD, NEG, // ([a], b)
  BAND, BOR, BXOR, BNOT, RSHIFT, LSHIFT, // (a, b)
  BGET, // (num, bit)
  BSET, // (num, bit)
  BCLR, // (num, bit)
  AND, OR, NOT, XOR, // ([a], b)
  GT, LT, GTE, LTE, EQ, NEQ, // (a, b)
  ISTC, ISFC, IST, ISF, JMP, // ([cond], jump)
  CALL, // (out/in, jump)
  END, // return
  DUMP,
} opcode;

const char* names[] = {
  "DROP",
  "DUP",
  "DUP2",
  "OVER",
  "SWAP",
  "ROT",
  "ADD", "SUB", "MUL", "DIV", "MOD", "NEG",
  "BAND", "BOR", "BXOR", "BNOT", "RSHIFT", "LSHIFT",
  "BGET",
  "BSET",
  "BCLR",
  "AND", "OR", "NOT", "XOR",
  "GT", "LT", "GTE", "LTE", "EQ", "NEQ",
  "ISTC", "ISFC", "IST", "ISF", "JMP",
  "CALL",
  "END",
  "DUMP",
};

int slots[32];
int* top = slots - 1;

void dump() {
  printf("stack:");
  int* i;
  for (i = slots; i <= top; i++) {
    printf(" %d", *i);
  }
  printf("\n");
}

#define Int16(val) ((uint16_t)(val) & 0xff), ((uint16_t)(val) >> 8)

static unsigned char code[] = {
  1, 2, ADD, 3, 4, ADD, MUL,
  CALL, 0x12, Int16(1),
  END,
  ISTC, Int16(5),
  42, 15,
  JMP, Int16(1),
  30, END
};

int eval(unsigned char* pc) {
  while (1) {
    dump();
    if (*pc < 0x80) {
      printf("%td: ", pc - code);
      *++top = *pc & 0x3f;
      if (*pc++ & 0x40) {
        do {
          *top = (*top << 7) | (*pc & 0x7f);
        } while (*pc++ & 0x80);
      }
      printf("%d\n", *top);
      continue;
    }
    printf("%td: %s\n", pc - code, names[*pc - 128]);
    switch ((opcode)*pc++) {
      case DROP: top--; continue;
      case DUP: top++; *top = *(top - 1); continue;
      case DUP2: {
        top += 2;
        *top = *(top - 2);
        *(top - 1) = *(top - 3);
        continue;
      }
      case OVER: top++; *top = *(top - 2); continue;
      case SWAP: {
        int temp = *top;
        *top = *(top - 1);
        *(top - 1) = temp;
        continue;
      }
      case ROT: {
        int temp = *(top - 2);
        *(top - 2) = *(top -  1);
        *(top - 1) = temp;
        *top = temp;
        continue;
      }
      case ADD: top--; *top += *(top + 1); continue;
      case SUB: top--; *top -= *(top + 1); continue;
      case MUL: top--; *top *= *(top + 1); continue;
      case DIV: top--; *top /= *(top + 1); continue;
      case MOD: top--; *top %= *(top + 1); continue;
      case NEG: *top = -*top; continue;
      case BAND: top--; *top &= *(top + 1); continue;
      case BOR:  top--; *top |= *(top + 1); continue;
      case BXOR: top--; *top ^= *(top + 1); continue;
      case RSHIFT: top--; *top >>= *(top + 1); continue;
      case LSHIFT: top--; *top <<= *(top + 1); continue;
      case BNOT: *top = ~*top; continue;
      case BGET: top--; *top = !!(*top & (1 << *(top + 1))); continue;
      case BSET: top--; *top |= 1 << *(top + 1); continue;
      case BCLR: top--; *top &= ~(1 << *(top + 1)); continue;
      case AND: top--; *top = *top ? *(top + 1) : 0; continue;
      case OR:  top--; *top = *top ? *top : *(top + 1); continue;
      case XOR: top--; *top = *top ?
        (*(top + 1) ? 0 : *top) :
        (*(top + 1) ? *(top + 1) : 0);
        continue;
      case NOT: *top = !*top; continue;
      case GT: top--; *top = *top > *(top + 1); continue;
      case LT: top--; *top = *top < *(top + 1); continue;
      case GTE: top--; *top = *top >= *(top + 1); continue;
      case LTE: top--; *top = *top <= *(top + 1); continue;
      case EQ:  top--; *top = *top == *(top + 1); continue;
      case NEQ: top--; *top = *top != *(top + 1); continue;
      case ISTC:
        printf("JUMP %d\n", *(int16_t*)pc);
        if (*top) pc += *(int16_t*)pc;
        pc += 2;
        continue;
      case ISFC:
        if (*top) pc += *(int16_t*)pc;
        pc += 2;
        continue;
      case IST:
        if (*top) pc += *(int16_t*)pc;
        pc += 2;
        top--;
        continue;
      case ISF:
        if (!*top) pc += *(int16_t*)pc;
        pc += 2;
        top--;
        continue;
      case JMP: pc += *(int16_t*)pc + 2; continue;
      case CALL: {
        char in = *pc & 0xf;
        int* end = top - (*pc >> 4) + in;
        pc += 3;
        eval(pc + *(int16_t*)(pc - 2));
        if (top < end) return -1;
        if (top > end ) {
          top -= in; end -= in;
          while (in--) *++end = *++top;
          top = end;
        }
        continue;
      }
      case DUMP: dump(); continue;
      case END: return 0;
    }
  }
}

int main() {
  int ret = eval(code);
  dump();
  if (ret) printf("Error in eval!\n");
  return ret;
}
