#include "uscript.h"

#ifdef ARDUINO
#include "Arduino.h"
#endif

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
  "PM",
  "DW",
  "DR",
  "AW",
  "AR",
  "SW",
  "SR",
  "DELAY",
  "UDELAY",
  "SRAND",
  "RAND",
  "ISTC", "ISFC", "IST", "ISF", "JMP",
  "CALL",
  "END",
  "DUMP",
};

// From http://inglorion.net/software/deadbeef_rand/
static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;
uint32_t deadbeef_rand() {
  deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
  deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
  return deadbeef_seed;
}
void deadbeef_srand(uint32_t x) {
  deadbeef_seed = x;
  deadbeef_beef = 0xdeadbeef;
}

static int stack[32];
static int* top = stack - 1;

void dump() {
  printString("stack:");
  int* i;
  for (i = stack; i <= top; ++i) {
    printString(" ");
    printInt(*i);
  }
  printString("\r\n");
}

void reset() {
  top = stack - 1;
}
int eval(unsigned char* pc) {
  while (1) {
    dump();
    if (*pc < 0x80) {
      *++top = *pc & 0x3f;
      if (*pc++ & 0x40) {
        do {
          *top = (*top << 7) | (*pc & 0x7f);
        } while (*pc++ & 0x80);
      }
      continue;
    }
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
      case PM: pinMode(*(top - 1), *(top)); top -= 2; continue;
      case DW: digitalWrite(*(top - 1), *(top)); top -= 2; continue;
      case DR: *top = digitalRead(*(top)); continue;
      case AW: analogWrite(*(top - 1), *(top)); top -= 2; continue;
      case AR: *top = analogRead(*(top)); continue;
      case SW:
        slotWrite((*(top - 1) >> 8) & 0xff, *(top - 1) & 0xff, *(top));
        top -= 2; continue;
      case SR: slotRead(*(top)); top--; continue;
      case DELAY: delay(*top); top--; continue;
      case UDELAY: delayMicroseconds(*top); top--; continue;
      case RAND: *top = deadbeef_rand() %*top; continue;
      case SRAND: deadbeef_srand(*top--); continue;
      case ISTC:
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
