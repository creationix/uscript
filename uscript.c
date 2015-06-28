#include <stdint.h>

enum opcodes {
  /* User Programs
  OP_DEF = 128, OP_RM, OP_CALL, OP_RUN, */
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* control flow */
  OP_IF, OP_ELIF, OP_ELSE, OP_MATCH, OP_WHEN, OP_WHILE, OP_DO,
  /* logic (short circuit and/or) */
  OP_NOT, OP_AND, OP_OR, OP_XOR,
  /* bitwise logic */
  OP_BNOT, OP_BAND, OP_BOR, OP_BXOR,
  OP_LSHIFT, OP_RSHIFT,
  /* comparison */
  OP_EQ, OP_NEQ, OP_GTE, OP_LTE, OP_GT, OP_LT,
  /* math */
  OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
  /* events */
  /*OP_WAIT, OP_ON, */
  /* timer */
  /*OP_DELAY, OP_TIMER, */
  /* io */
  /*OP_PM, OP_DW, OP_PW, OP_DR, OP_AR,*/
  /* neopixel */
  /*OP_NP, OP_NPW, OP_RGB, OP_HSV, OP_HCL, OP_UPDATE, */
  /* servo */
  /*OP_SERVO, OP_MOVE, */
  /* tone */
  /*OP_TONE,*/
};

static int32_t vars[26];

static const uint8_t* skip(const uint8_t* pc) {
  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {

    // Need to read the length header to skip do
    case OP_DO: {
      uint8_t count = *pc++;
      while (count--) pc = skip(pc);
      return pc;
    }

    // Opcodes that consume one opcode
    case OP_GET: case OP_INCR: case OP_DECR:
      return pc + 1;

    // Opcodes that consume one opcode and one expression
    case OP_SET: return skip(pc + 1);


    // Opcodes that consume one expression
    case OP_NOT: case OP_BNOT: case OP_NEG:
    case OP_MATCH: case OP_IF: case OP_ELSE:
      return skip(pc);

    // Opcodes that consume two expressions
    case OP_WHILE: case OP_ELIF: case OP_WHEN:
    case OP_AND: case OP_OR: case OP_XOR:
    case OP_BAND: case OP_BOR: case OP_BXOR: case OP_LSHIFT: case OP_RSHIFT:
    case OP_EQ: case OP_NEQ: case OP_GTE: case OP_LTE: case OP_GT: case OP_LT:
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
      return skip(skip(pc));

  }

  // Otherwise it's a variable length encoded integer.
  else {
    if (!(*pc++ & 0x40)) return pc;
    if (!(*pc++ & 0x80)) return pc;
    if (!(*pc++ & 0x80)) return pc;
    if (!(*pc++ & 0x80)) return pc;
    return pc + 1;
  }
}

#define binop(code, op) \
  case code: { \
    int32_t a, b; \
    pc = eval(pc, &a); \
    pc = eval(pc, &b); \
    *res = a op b; \
    return pc; \
  }

#define unop(code, op) \
  case code: { \
    pc = eval(pc, res); \
    *res = op*res; \
    return pc; \
  }

static const uint8_t* eval(const uint8_t* pc, int32_t* res) {

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      return 0;

    case OP_SET: {
      uint8_t idx = *pc++;
      pc = eval(pc, res);
      vars[idx] = *res;
      return pc;
    }
    case OP_GET:
      *res = vars[*pc];
      return pc + 1;

    case OP_INCR:
      *res = ++vars[*pc];
      return pc + 1;

    case OP_DECR:
      *res = --vars[*pc];
      return pc + 1;

    case OP_IF: {
      int32_t cond;
      char done = 0;
      *res = 0;
      pc = eval(pc, &cond);
      if (cond) {
        done = 1;
        pc = eval(pc, res);
      }
      else pc = skip(pc);
      while (*pc == OP_ELIF) {
        pc++;
        if (done) {
          pc = skip(skip(pc));
        }
        else {
          pc = eval(pc, &cond);
          if (cond) {
            done = 1;
            pc = eval(pc, res);
          }
          else pc = skip(pc);
        }
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(pc);
        else pc = eval(pc, res);
      }
      return pc;
    }

    case OP_MATCH: {
      int32_t val, cond;
      char done = 0;
      *res = 0;
      pc = eval(pc, &val);
      while (*pc == OP_WHEN) {
        pc++;
        if (done) {
          pc = skip(pc); // cond
          pc = skip(pc); // val
        }
        pc = eval(pc, &cond);
        if (cond == val) {
          done = 1;
          pc = eval(pc, res);
        }
        else pc = skip(pc);
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(pc);
        else pc = eval(pc, res);
      }
      return pc;
    }

    case OP_WHILE: {
      const uint8_t* c = pc;
      int32_t cond;
      *res = 0;
      while (pc = eval(c, &cond), cond) {
        eval(pc, res);
      }
      return skip(pc);
    }

    case OP_DO: {
      uint8_t count = *pc++;
      *res = 0;
      while (count--) pc = eval(pc, res);
      return pc;
    }

    unop(OP_NOT, !)
    case OP_AND:
      pc = eval(pc, res);
      if (*res) pc = eval(pc, res);
      else pc = skip(pc);
      return pc;
    case OP_OR:
      pc = eval(pc, res);
      if (*res) pc = skip(pc);
      else pc = eval(pc, res);
      return pc;
    case OP_XOR: {
      int32_t a, b;
      pc = eval(pc, &a);
      pc = eval(pc, &b);
      *res = a ? (b ? 0 : a) : (b ? b : 0);
      return pc;
    }


    unop(OP_BNOT, ~)
    binop(OP_BAND, &)
    binop(OP_BOR, |)
    binop(OP_BXOR, ^)
    binop(OP_LSHIFT, <<)
    binop(OP_RSHIFT, >>)

    binop(OP_EQ, ==)
    binop(OP_NEQ, !=)
    binop(OP_GTE, >=)
    binop(OP_LTE, <=)
    binop(OP_GT, >)
    binop(OP_LT, <)

    unop(OP_NEG, -)
    binop(OP_ADD, +)
    binop(OP_SUB, -)
    binop(OP_MUL, *)
    binop(OP_DIV, /)
    binop(OP_MOD, %)

  }

  // Otherwise it's a variable length encoded integer.
  else {
    *res = *pc & 0x3f;
    if (!(*pc++ & 0x40)) return pc;
    *res |= (*pc & 0x7f) << 6;
    if (!(*pc++ & 0x80)) return pc;
    *res |= (*pc & 0x7f) << 13;
    if (!(*pc++ & 0x80)) return pc;
    *res |= (*pc & 0x7f) << 20;
    if (!(*pc++ & 0x80)) return pc;
    *res |= (*pc & 0x1f) << 27;
    return pc + 1;
  }
}
