
#ifdef OP_LOG
#include <stdio.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#define OP_CODES(XX) \
/* User Programs \
XX(DEF) XX(RM) XX(CALL) XX(RUN) */\
/* variables */\
XX(SET) XX(GET) XX(INCR) XX(DECR) \
/* control flow */\
XX(IF) XX(ELIF) XX(ELSE) XX(MATCH) XX(WHEN) XX(WHILE) XX(DO)\
/* logic (short circuit and/or) */\
XX(NOT) XX(AND) XX(OR) \
/* bitwise logic */\
XX(BNOT) XX(BAND) XX(BOR) XX(BXOR) \
/* comparison */\
XX(EQ) XX(NEQ) XX(GTE) XX(LT) \
/* math */\
XX(NEG) XX(ADD) XX(SUB) XX(MUL) XX(DIV) XX(MOD) /*XX(RAND)*/ \
/* events */\
/*XX(WAIT) XX(ON) */\
/* timer */\
XX(DELAY) /*XX(TIMER) */\
/* io */\
/*XX(PM) XX(DW) XX(PW) XX(DR) XX(AR)*/ \
/* neopixel */\
/*XX(NP) XX(NPW) XX(RGB) XX(HSV) XX(HCL) XX(UPDATE) */\
/* servo */\
/*XX(SERVO) XX(MOVE) */\
/* tone */\
/*XX(TONE)*/

enum opcodes {
  OP_MAX_NUM = 127,
  #define XX(name) OP_ ## name,
    OP_CODES(XX)
  #undef XX
};


#ifdef OP_LOG
static const char* op_name(enum opcodes code) {
  switch (code) {
    case OP_MAX_NUM: return "MAX_NUM";
    #define XX(name) case OP_ ## name: return #name;
      OP_CODES(XX)
    #undef XX
  }
  return "NUMBER";
}
#endif

static int32_t vars[26];

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

const uint8_t* skip(const uint8_t* pc) {
  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    #ifdef OP_LOG
    printf("SOP=%02x - %s\n", *pc, op_name(*pc));
    #endif

    switch ((enum opcodes)*pc++) {
    case OP_MAX_NUM: assert(0);
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      assert(0);

    case OP_IF:
      pc = skip(pc); // cond
      while (*pc == OP_ELIF) pc = skip(skip(++pc)); // cond/val
      if (*pc == OP_ELSE) pc = skip(++pc); // val
      return pc;

    case OP_MATCH:
      pc = skip(pc); // cond
      while (*pc == OP_WHEN) pc = skip(skip(++pc)); // cond/val
      if (*pc == OP_ELSE) pc = skip(++pc); // val
      return pc;

    case OP_DO: {
      uint8_t count = *pc++;
      while (count--) pc = skip(pc);
      return pc;
    }

    case OP_SET: return skip(pc + 1);
    case OP_GET:
    case OP_INCR:
    case OP_DECR:
      return pc + 1;

    case OP_NOT:
    case OP_BNOT:
    case OP_NEG:
    case OP_DELAY:
      return skip(pc);

    case OP_WHILE:
    case OP_AND:
    case OP_OR:
    case OP_BAND:
    case OP_BOR:
    case OP_BXOR:
    case OP_EQ:
    case OP_NEQ:
    case OP_GTE:
    case OP_LT:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD: {
      return skip(skip(pc));
    }

    }
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

const uint8_t* eval(const uint8_t* pc, int* res) {

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) {
    #ifdef OP_LOG
    printf("OP=%02x - %s\n", *pc, op_name(*pc));
    #endif

    switch ((enum opcodes)*pc++) {
    case OP_MAX_NUM: assert(0);
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      assert(0);

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
      bool done = false;
      *res = 0;
      pc = eval(pc, &cond);
      if (cond) {
        done = true;
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
            done = true;
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
      bool done = false;
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
          done = true;
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

    //
    // case OP_LOOP:
    //
    // case OP_BREAK:

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

    unop(OP_BNOT, ~)
    binop(OP_BAND, &)
    binop(OP_BOR, |)
    binop(OP_BXOR, ^)

    binop(OP_EQ, ==)
    binop(OP_NEQ, !=)
    binop(OP_GTE, >=)
    binop(OP_LT, <)

    unop(OP_NEG, -)
    binop(OP_ADD, +)
    binop(OP_SUB, -)
    binop(OP_MUL, *)
    binop(OP_DIV, /)
    binop(OP_MOD, %)

    case OP_DELAY:
      pc = eval(pc, res);
      usleep(*res * 1000);
      return pc;
  }
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
