#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

int64_t slots[16];
uint8_t* labels[16];
uint8_t* pc;

enum opcode {
  // Consume one expression and write to register slot
  W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14, W15,
  // Set program labels
  L0, L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, L13, L14, L15,
  // Jump to program labels
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15,
  // Conditional Jumps, accept two 4-bit arguments for register and labels
  IS, ISN,
  // Statement math
  RADD, RSUB, RISUB, RMUL, RDIV, RIDIV, RMOD, RIMOD, RNEG, INCR, DECR,
  // Print takes a single expression and prints it.
  PRINT,
  // end
  RETURN, END,
  // // Socket write (address, port, value)
  // SW,
  // // Digital Write, Analog Write (port, value)
  // DW, AW,
};

// Expression tree nodes
enum node {
  // 0mxxxxxx mxxxxxxx: variable length unsigned integer literal.
  // Read register slots 0 - 15
  R0 = 128, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
  // Basic math
  ADD, SUB, MUL, DIV, MOD, NEG,
  // Basic comparison
  EQ, NEQ, GT, LT, GTE, LTE,
  // Basic logic
  AND, OR, XOR, NOT,
  // Conditionals
  IF, ELIF, ELSE,
  // // Read from virtual slots (network ports)
  // SR,
  // // Read from GPIO port (digital value)
  // DR,
  // // Read from GPIO port (analog value)
  // AR,
};

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
    case ELIF: case ELSE: assert(0);
  }
}

int64_t eval() {
  if (*pc < 0x80) {
    int64_t num = *pc & 0x3f;
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
      int64_t a = eval();
      if (a) return eval();
      skip(); return a;
    }
    case OR: {
      int64_t a = eval();
      if (!a) return eval();
      skip(); return a;
    }
    case XOR: {
      int64_t a, b;
      a = eval();
      b = eval();
      return a ? (!b ? a : 0) : (b ? b : 0);
    }
    case NOT: return !eval();
    case IF: {
      int64_t r = 0;
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
    case ELIF: case ELSE: assert(0);

  }
  return 0;
}

// int64_t z = 0;
void exec() {
  while (1) {
    // if (!(z++ % 30000000)) {
    //   printf("%"PRId64" %"PRId64"\n", slots[0], slots[1]);
    // }
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
        continue;
      case ISN:
        if (!slots[*pc >> 4]) pc = labels[*pc & 0xf];
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
      case PRINT:
        printf("%"PRId64"\n", eval());
        continue;
      case RETURN:
        // start = *pc >> 4
        // end = *pc & 0xf
        // Copy values to return stack
        return;
      case END: return;
    }
  }
}


int main() {
  slots[0] = 10;
  slots[1] = 20;
  pc = (uint8_t[]){
    ADD, R0, R1
  };
  printf("%"PRId64"\n", eval());
  pc = (uint8_t[]){
    W0, 0x43, 0xdc, 0xeb, 0x94, 0x00, // a = 1000000000
    W1, 0x0,                          // b = 0
    L0,                               // :start:
    RADD, 0x10,                       // b += a
    DECR, 0x01,                       // a -= 1
    IS, 0x00,                         // if a goto :start:
    PRINT, R1,                        // print b
    END,
  };
  exec();

  return 0;
}
