#include "uscript.h"

int add_string(const char* string) {
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

void skip() {
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
    case NEG: case NOT:
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

integer eval() {
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
    case R0: return slots[0];
    case R1: return slots[1];
    case R2: return slots[2];
    case R3: return slots[3];
    case R4: return slots[4];
    case R5: return slots[5];
    case R6: return slots[6];
    case R7: return slots[7];
    case R8: return slots[8];
    case R9: return slots[9];
    case R10: return slots[10];
    case R11: return slots[11];
    case R12: return slots[12];
    case R13: return slots[13];
    case R14: return slots[14];
    case R15: return slots[15];
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

void exec() {
  while (1) {
    switch ((enum opcode) *pc++) {
      case W0: slots[0] = eval(); continue;
      case W1: slots[1] = eval(); continue;
      case W2: slots[2] = eval(); continue;
      case W3: slots[3] = eval(); continue;
      case W4: slots[4] = eval(); continue;
      case W5: slots[5] = eval(); continue;
      case W6: slots[6] = eval(); continue;
      case W7: slots[7] = eval(); continue;
      case W8: slots[8] = eval(); continue;
      case W9: slots[9] = eval(); continue;
      case W10: slots[10] = eval(); continue;
      case W11: slots[11] = eval(); continue;
      case W12: slots[12] = eval(); continue;
      case W13: slots[13] = eval(); continue;
      case W14: slots[14] = eval(); continue;
      case W15: slots[15] = eval(); continue;

      case L0: labels[0] = pc; continue;
      case L1: labels[1] = pc; continue;
      case L2: labels[2] = pc; continue;
      case L3: labels[3] = pc; continue;
      case L4: labels[4] = pc; continue;
      case L5: labels[5] = pc; continue;
      case L6: labels[6] = pc; continue;
      case L7: labels[7] = pc; continue;
      case L8: labels[8] = pc; continue;
      case L9: labels[9] = pc; continue;
      case L10: labels[10] = pc; continue;
      case L11: labels[11] = pc; continue;
      case L12: labels[12] = pc; continue;
      case L13: labels[13] = pc; continue;
      case L14: labels[14] = pc; continue;
      case L15: labels[15] = pc; continue;

      case J0: pc = labels[0]; continue;
      case J1: pc = labels[1]; continue;
      case J2: pc = labels[2]; continue;
      case J3: pc = labels[3]; continue;
      case J4: pc = labels[4]; continue;
      case J5: pc = labels[5]; continue;
      case J6: pc = labels[6]; continue;
      case J7: pc = labels[7]; continue;
      case J8: pc = labels[8]; continue;
      case J9: pc = labels[9]; continue;
      case J10: pc = labels[10]; continue;
      case J11: pc = labels[11]; continue;
      case J12: pc = labels[12]; continue;
      case J13: pc = labels[13]; continue;
      case J14: pc = labels[14]; continue;
      case J15: pc = labels[15]; continue;
      case IS:
        if (slots[*pc >> 4]) pc = labels[*pc & 0xf];
        else ++pc;
        continue;
      case ISN:
        if (!slots[*pc >> 4]) pc = labels[*pc & 0xf];
        else ++pc;
        continue;
      case RADD:
        slots[*pc >> 4] += slots[*pc & 0xf];
        ++pc; continue;
      case RSUB:
        slots[*pc >> 4] -= slots[*pc & 0xf];
        ++pc; continue;
      case RISUB:
        slots[*pc >> 4] = slots[*pc & 0xf] - slots[*pc >> 4];
        ++pc; continue;
      case RMUL:
        slots[*pc >> 4] *= slots[*pc & 0xf];
        ++pc; continue;
      case RDIV:
        slots[*pc >> 4] /= slots[*pc & 0xf];
        ++pc; continue;
      case RIDIV:
        slots[*pc >> 4] = slots[*pc & 0xf] / slots[*pc >> 4];
        ++pc; continue;
      case RMOD:
        slots[*pc >> 4] %= slots[*pc & 0xf];
        ++pc; continue;
      case RIMOD:
        slots[*pc >> 4] = slots[*pc & 0xf] % slots[*pc >> 4];
        ++pc; continue;
      case RNEG:
        slots[*pc >> 4] = -slots[*pc & 0xf];
        ++pc; continue;
      case INCR:
        slots[*pc >> 4] += *pc & 0xf;
        ++pc; continue;
      case DECR:
        slots[*pc >> 4] -= *pc & 0xf;
        ++pc; continue;
      case RETURN:
        // start = *pc >> 4
        // end = *pc & 0xf
        // Copy values to return stack
        return;
      case END: return;
    }
  }
}
