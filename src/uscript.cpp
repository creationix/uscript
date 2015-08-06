#include "uscript.h"

// String data pool
char strings[SIZE_STRINGS];
// User function data pool
unsigned char* funcs[MAX_FUNCS];
// value registers
integer slots[MAX_VALUES];
// jump traget register
unsigned char* labels[MAX_LABELS];

// Current value registers
integer* d;
// Current label registers
unsigned char** l;
// Current program counter
unsigned char* pc;

#ifdef ARDUINO
#include <Arduino.h>
void print(int value) {
  Serial.println(value);
}
#else
static integer digitalRead(integer pin) { return 0; }
static integer analogRead(integer pin) { return 0; }
static void digitalWrite(integer pin, integer value) { }
static void analogWrite(integer pin, integer value) { }
static void pinMode(integer pin, integer mode) { }
static void delay(integer ms) { }
static void delayMicroseconds(integer us) { }
#endif

void uscript_print(integer number);
void uscript_prints(const char* string);

int put_string(const char* string) {
  // Count length of input string
  int len;
  {
    const char* end = string;
    while (*end) end++;
    len = end - string;
  }
  // Empty strings are an error
  if (!len) return -1;

  // Search for existing match
  char* s = strings;
  int index = 0;
  while (*s) {
    int i = 0;
    while (string[i] && string[i] == s[i]) i++;
    if (i == len && s[i] == 0) return index;
    while (*s++);
    index++;
  }
  // Out of space error
  if (s + len + 1 >= strings + SIZE_STRINGS) return -1;
  {
    // Copy the new string
    int i;
    for (i = 0; i < len; i++) {
      s[i] = string[i];
    }
  }
  return index;
}

const char* get_string(int index) {
  char* s = strings;
  while (index--) {
    if (!*s) return 0;
    while (*s) s++;
    s++;
  }
  return s;
}

static void skip() {
  if (*pc < 0x80) {
    if (*pc++ & 0x40) {
      while (*pc++ & 0x80);
    }
    return;
  }
  switch ((enum node) *pc++) {
    // Nodes that consume no expressions
    case R0: case R1: case R2: case R3: case R4: case R5: case R6: case R7:
    case R8: case R9: case R10: case R11: case R12: case R13: case R14: case R15:
      return;
    // Nodes that consume one expression
    case NEG: case NOT: case DR: case AR:
      skip(); return;
    // Nodes that consume two expressions
    case ADD: case SUB: case MUL: case DIV: case MOD:
    case EQ: case NEQ: case GT: case LT: case GTE: case LTE:
    case AND: case OR: case XOR:
      skip(); skip(); return;
    case IF:
      skip(); skip(); // condition / body
      while (*pc == ELIF) { ++pc; skip(); skip(); } // ELIF / condition / body
      if (*pc == ELSE) { ++pc; skip(); } // ELSE / body
    case ELIF: case ELSE:
      // TODO: This should never happen, fail somehow?
      return;
  }
}

static integer eval() {
  if (*pc < 0x80) {
    integer num = *pc & 0x3f;
    if (*pc++ & 0x40) {
      do {
        num = (num << 7) | (*pc & 0x7f);
      } while (*pc++ & 0x80);
    }
    return num;
  }
  switch ((enum node) *pc++) {
    case R0: return d[0];
    case R1: return d[1];
    case R2: return d[2];
    case R3: return d[3];
    case R4: return d[4];
    case R5: return d[5];
    case R6: return d[6];
    case R7: return d[7];
    case R8: return d[8];
    case R9: return d[9];
    case R10: return d[10];
    case R11: return d[11];
    case R12: return d[12];
    case R13: return d[13];
    case R14: return d[14];
    case R15: return d[15];
    case DR: return digitalRead(eval());
    case AR: return analogRead(eval());
    case ADD: return eval() + eval();
    case SUB: return eval() - eval();
    case MUL: return eval() * eval();
    case DIV: return eval() / eval();
    case MOD: return eval() % eval();
    case NEG: return -eval();
    case EQ: return eval() == eval();
    case NEQ: return eval() != eval();
    case GT: return eval() > eval();
    case LT: return eval() < eval();
    case GTE: return eval() >= eval();
    case LTE: return eval() <= eval();
    case AND: {
      integer a = eval();
      if (a) return eval();
      skip(); return a;
    }
    case OR: {
      integer a = eval();
      if (!a) return eval();
      skip(); return a;
    }
    case XOR: {
      integer a, b;
      a = eval();
      b = eval();
      return a ? (!b ? a : 0) : (b ? b : 0);
    }
    case NOT: return !eval();
    case IF: {
      integer r = 0;
      while (1) {
        if (eval()) {
          r = eval();
          break;
        }
        skip();
        if (*pc == ELIF) {
          ++pc; continue;
        }
        if (*pc == ELSE) {
          ++pc;
          return eval();
        }
        return 0;
      }
      while (*pc == ELIF) { ++pc; skip(); skip(); }
      if (*pc == ELSE) { ++pc; skip(); }
      return r;
    }
    case ELIF: case ELSE:
      // TODO: This should never happen, fail somehow?
      return -1;
  }
  return 0;
}

static void skip_to_end() {
  int depth = 1;
  while (depth) switch((enum opcode) *pc++) {
    // Statements that don't consume anything
    case L0: case L1: case L2: case L3: case L4: case L5: case L6: case L7:
    case L8: case L9: case L10: case L11: case L12: case L13: case L14: case L15:
    case J0: case J1: case J2: case J3: case J4: case J5: case J6: case J7:
    case J8: case J9: case J10: case J11: case J12: case J13: case J14: case J15:
      continue;
    // Statements that consume one byte
    case IS: case ISN:
    case RADD: case RSUB: case RISUB: case RMUL: case RDIV: case RIDIV:
    case RMOD: case RIMOD: case RNEG: case INCR: case DECR:
      pc++; continue;
    // Statements that consume two bytes
    case CALL:
      pc += 2; continue;
    // Statements that consume one expression
    case W0: case W1: case W2: case W3: case W4: case W5: case W6: case W7:
    case W8: case W9: case W10: case W11: case W12: case W13: case W14: case W15:
    case DELAY: case UDELAY: case PRINT: case PRINTS:
      skip(); continue;
    // Statements that consume two expressions
    case DW: case AW: case PM:
      skip(); skip(); continue;
    case DEF:
      pc++; depth++; continue;
    case END: depth--; continue;
  }
}

static void exec() {
  while (1) {
    switch ((enum opcode) *pc++) {
      case W0: d[0] = eval(); continue;
      case W1: d[1] = eval(); continue;
      case W2: d[2] = eval(); continue;
      case W3: d[3] = eval(); continue;
      case W4: d[4] = eval(); continue;
      case W5: d[5] = eval(); continue;
      case W6: d[6] = eval(); continue;
      case W7: d[7] = eval(); continue;
      case W8: d[8] = eval(); continue;
      case W9: d[9] = eval(); continue;
      case W10: d[10] = eval(); continue;
      case W11: d[11] = eval(); continue;
      case W12: d[12] = eval(); continue;
      case W13: d[13] = eval(); continue;
      case W14: d[14] = eval(); continue;
      case W15: d[15] = eval(); continue;

      case L0: l[0] = pc; continue;
      case L1: l[1] = pc; continue;
      case L2: l[2] = pc; continue;
      case L3: l[3] = pc; continue;
      case L4: l[4] = pc; continue;
      case L5: l[5] = pc; continue;
      case L6: l[6] = pc; continue;
      case L7: l[7] = pc; continue;
      case L8: l[8] = pc; continue;
      case L9: l[9] = pc; continue;
      case L10: l[10] = pc; continue;
      case L11: l[11] = pc; continue;
      case L12: l[12] = pc; continue;
      case L13: l[13] = pc; continue;
      case L14: l[14] = pc; continue;
      case L15: l[15] = pc; continue;

      case J0: pc = l[0]; continue;
      case J1: pc = l[1]; continue;
      case J2: pc = l[2]; continue;
      case J3: pc = l[3]; continue;
      case J4: pc = l[4]; continue;
      case J5: pc = l[5]; continue;
      case J6: pc = l[6]; continue;
      case J7: pc = l[7]; continue;
      case J8: pc = l[8]; continue;
      case J9: pc = l[9]; continue;
      case J10: pc = l[10]; continue;
      case J11: pc = l[11]; continue;
      case J12: pc = l[12]; continue;
      case J13: pc = l[13]; continue;
      case J14: pc = l[14]; continue;
      case J15: pc = l[15]; continue;

      case DW: {
        integer pin = eval();
        integer value = eval();
        digitalWrite(pin, value);
        continue;
      }
      case AW: {
        integer pin = eval();
        integer value = eval();
        analogWrite(pin, value);
        continue;
      }
      case PM: {
        integer pin = eval();
        integer mode = eval();
        pinMode(pin, mode);
        continue;
      }
      case DELAY: delay(eval()); continue;
      case UDELAY: delayMicroseconds(eval()); continue;
      case PRINT: uscript_print(eval()); continue;
      case PRINTS: uscript_prints(get_string(eval())); continue;
      case IS:
        if (d[*pc >> 4]) pc = l[*pc & 0xf];
        else ++pc;
        continue;
      case ISN:
        if (!d[*pc >> 4]) pc = l[*pc & 0xf];
        else ++pc;
        continue;
      case RADD:
        d[*pc >> 4] += d[*pc & 0xf];
        ++pc; continue;
      case RSUB:
        d[*pc >> 4] -= d[*pc & 0xf];
        ++pc; continue;
      case RISUB:
        d[*pc >> 4] = d[*pc & 0xf] - d[*pc >> 4];
        ++pc; continue;
      case RMUL:
        d[*pc >> 4] *= d[*pc & 0xf];
        ++pc; continue;
      case RDIV:
        d[*pc >> 4] /= d[*pc & 0xf];
        ++pc; continue;
      case RIDIV:
        d[*pc >> 4] = d[*pc & 0xf] / d[*pc >> 4];
        ++pc; continue;
      case RMOD:
        d[*pc >> 4] %= d[*pc & 0xf];
        ++pc; continue;
      case RIMOD:
        d[*pc >> 4] = d[*pc & 0xf] % d[*pc >> 4];
        ++pc; continue;
      case RNEG:
        d[*pc >> 4] = -d[*pc & 0xf];
        ++pc; continue;
      case INCR:
        d[*pc >> 4] += *pc & 0xf;
        ++pc; continue;
      case DECR:
        d[*pc >> 4] -= *pc & 0xf;
        ++pc; continue;
      case DEF: {
        int index = *pc++;
        unsigned char* start = pc;
        skip_to_end();
        if (index < 0 || index >= MAX_FUNCS) return;
        free(funcs[index]);
        int len = pc - start;
        funcs[index] = (unsigned char*)malloc(len);
        memcpy(funcs[index], start, len);
        continue;
      }
      case CALL: {
        // Index of function to call
        int index = *pc++;
        // slot offset
        int offset = *pc++;
        integer* top = d;
        unsigned char* ret = pc;
        d += offset;
        pc = funcs[index];
        exec();
        d = top;
        pc = ret;
        continue;
      }
      case END: return;
    }
  }
}

void uscript_setup() {
  d = slots;
  l = labels;
}

integer* uscript_run(unsigned char* code) {
  pc = code;
  exec();
  return d;
}
