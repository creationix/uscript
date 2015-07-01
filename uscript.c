#include <stdint.h>
#include <string.h>
#include <assert.h>

// #define ARDUINO
// #include "rpi-io.c"

#ifdef ARDUINO
  #include "Arduino.h"
  #define var int32_t
  #define OP_WIRING
#else
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #define var int64_t
  #ifdef BCM2708_PERI_BASE
    #define OP_WIRING
  #endif
#endif

void (*print_fn)(var);

// global variables.
static var vars[26];

// global functions/lambdas/subroutines (heap allocated)
static uint8_t* stubs[26];

// Active event listeners
struct event {
  struct event* next;
  uint8_t code[]; // Contains condition and block as two expressions
};
static struct event* queue;

enum opcodes {
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* control flow */
  OP_IF, OP_ELIF, OP_ELSE, OP_MATCH, OP_WHEN, OP_WHILE, OP_DO, OP_FOR,
  /* logic (short circuit and/or) */
  OP_NOT, OP_AND, OP_OR, OP_XOR,
  /* bitwise logic */
  OP_BNOT, OP_BAND, OP_BOR, OP_BXOR, OP_LSHIFT, OP_RSHIFT,
  /* comparison */
  OP_EQ, OP_NEQ, OP_GTE, OP_LTE, OP_GT, OP_LT,
  /* math */
  OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
  /* misc */
  OP_DELAY, OP_RAND, OP_PRINT,
  #ifdef OP_WIRING
  /* io */
  OP_PM, OP_DW, OP_AW, OP_DR, OP_AR,
  #endif
  /* stubs */
  OP_DEF, OP_RM, OP_RUN, OP_WAIT, /*OP_TIMEOUT,*/
};

static const char* op_names =
  "SET\0GET\0INCR\0DECR\0"
  "IF\0ELIF\0ELSE\0MATCH\0WHEN\0WHILE\0DO\0FOR\0"
  "NOT\0AND\0OR\0XOR\0"
  "BNOT\0BAND\0BOR\0BXOR\0"
  "LSHIFT\0RSHIFT\0"
  "EQ\0NEQ\0GTE\0LTE\0GT\0LT\0"
  "NEG\0ADD\0SUB\0MUL\0DIV\0MOD\0"
  "DELAY\0RAND\0PRINT\0"
  #ifdef OP_WIRING
  "PM\0DW\0AW\0DR\0AR\0"
  #endif
  "DEF\0RM\0RUN\0WAIT\0"
  "\0"
;

const char* op_to_name(enum opcodes op) {
  int count = op - 128;
  const char* name = op_names;
  while (count--) while (*name++);
  return name;
}

static int name_to_op(const char* name, int len) {
  int op = 128;
  const char* list = op_names;
  while (*list) {
    int i;
    for (i = 0; *list == name[i]; list++) {
      if (++i == len && !*(list + 1)) return op;
    }
    while (*list++);
    op++;
  }
  return 0;
}

int compile(uint8_t* program) {
  uint8_t *cc = program,
          *pc = program;
  while (*cc) {
    // Skip white space
    if (*cc == ' ' || *cc == '\n') cc++;

    // Integer parsing
    else if (*cc >= '0' && *cc <= '9') {

      // Parse the decimal ascii number
      var val = 0;
      do {
        val = val * 10 + *cc++ - '0';
      } while (*cc >= 0x30 && *cc < 0x40);

      // Make sure it ended on a word boundary
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;

      // Encode as a variable length binary integer.
      if (val < 0x40) {
        *pc++ = val & 0x3f;
      }
      else {
        *pc++ = (val & 0x3f) | 0x40;
        val >>= 6;
        while (val) {
          if (val < 0x80) {
            *pc++ = val & 0x7f;
            break;
          }
          *pc++ = (val & 0x7f) | 0x80;
          val >>= 7;
        }
      }

      // printf("INTEGER %d\n", val);
    }

    // Variable parsing
    else if (*cc >= 'a' && *cc <= 'z') {

      #ifdef ARDUINO
        if (*cc == 'a' && *(cc + 1) >= '0' && *(cc + 1) <= '9') {
          cc++;
          *pc++ = *cc++ - '0' + A0;
          if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;
          continue;
        }
      #endif

      // Decode letters a-z as numbers 0-25
      uint8_t index = *cc++ - 'a';

      // Make sure it ended on a word boundary
      // Variables must be single digit.
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc;

      // Encode as simple integer.
      *pc++ = index;

      // printf("VARIABLE %c\n", index + 'a');
    }

    // Opcode parsing
    else if (*cc >= 'A' && *cc <= 'Z') {
      // Pre-scan the opcode to get it's length
      uint8_t *name = cc;
      do cc++; while (*cc >= 'A' && *cc <= 'Z');

      // Make sure it ended on a word boundary
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;

      // Look up the string in the table
      uint8_t op = name_to_op((char*)name, cc - name);

      // Handle non-matches
      if (op < 128) return program - name - 1;

      // Write as simple opcode integer
      *pc++ = op;

      // printf("OPCODE %s\n", op_to_name(op));
    }
    else {
      // Invalid character
      return program - cc - 1;
    }
  }
  return pc - program;
}

static uint8_t* skip(uint8_t* pc) {
  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {

    // Need to read the length header to skip DO
    case OP_DO: {
      uint8_t count = *pc++;
      while (count--) pc = skip(pc);
      return pc;
    }

    // Opcodes that consume one opcode
    case OP_GET: case OP_RM: case OP_RUN:
      return pc + 1;

    // Opcodes that consume one opcode and one expression
    case OP_SET: case OP_INCR: case OP_DECR: case OP_DEF:
      return skip(pc + 1);

    // Opcodes that consume one opcode and three expressions
    case OP_FOR:
      return skip(skip(skip(pc + 1)));

    // Opcodes that consume one expression
    case OP_NOT: case OP_BNOT: case OP_NEG:
    case OP_MATCH: case OP_IF: case OP_ELSE:
    case OP_DELAY: case OP_RAND: case OP_PRINT:
    #ifdef OP_WIRING
    case OP_DR: case OP_AR:
    #endif
      return skip(pc);

    // Opcodes that consume two expressions
    case OP_WHILE: case OP_ELIF: case OP_WHEN:
    case OP_AND: case OP_OR: case OP_XOR:
    case OP_BAND: case OP_BOR: case OP_BXOR: case OP_LSHIFT: case OP_RSHIFT:
    case OP_EQ: case OP_NEQ: case OP_GTE: case OP_LTE: case OP_GT: case OP_LT:
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
    #ifdef OP_WIRING
    case OP_PM: case OP_DW: case OP_AW:
    #endif
    case OP_WAIT:
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
    var a, b; \
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

uint8_t* eval(uint8_t* pc, var* res) {

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

    case OP_INCR: {
      var step;
      uint8_t idx = *pc++;
      pc = eval(pc, &step);
      *res = vars[idx] += step;
      return pc;
    }

    case OP_DECR: {
      var step;
      uint8_t idx = *pc++;
      pc = eval(pc, &step);
      *res = vars[idx] -= step;
      return pc;
    }

    case OP_IF: {
      var cond;
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
      var val, cond;
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
      uint8_t* c = pc;
      var cond;
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

    case OP_FOR: {
      uint8_t idx = *pc++;
      var start, end;
      pc = eval(pc, &start);
      pc = eval(pc, &end);
      uint8_t* body = pc;
      *res = 0;
      while (start <= end) {
        vars[idx] = start++;
        pc = eval(body, res);
      }
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
      var a, b;
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

    case OP_PRINT:
      pc = eval(pc, res);
      print_fn(*res);
      return pc;

    #ifndef ARDUINO

    case OP_DELAY: {
      pc = eval(pc, res);
      struct timeval t;
      t.tv_sec = *res / 1000;
      t.tv_usec = (*res % 1000) * 1000;
      select(0, NULL, NULL, NULL, &t);
      return pc;
    }

    case OP_RAND:
      pc = eval(pc, res);
      *res = rand() % *res;
      return pc;

    #else

    case OP_DELAY:
      pc = eval(pc, res);
      delay(*res);
      return pc;

    case OP_RAND:
      pc = eval(pc, res);
      *res = random(*res);
      return pc;

    case OP_PM: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      pinMode(pin, *res);
      return pc;
    }

    case OP_DW: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      digitalWrite(pin, *res);
      return pc;
    }

    case OP_AW: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      analogWrite(pin, *res);
      return pc;
    }

    case OP_DR: {
      var pin;
      pc = eval(pc, &pin);
      *res = digitalRead(pin);
      return pc;
    }

    case OP_AR: {
      var pin;
      pc = eval(pc, &pin);
      *res = analogRead(pin);
      return pc;
    }

    #endif
    #ifdef BCM2708_PERI_BASE

    case OP_PM: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      INP_GPIO(pin);
      if (*res) OUT_GPIO(pin);
      return pc;
    }

    case OP_DW: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      if (*res) GPIO_SET = 1 << pin;
      else GPIO_CLR = 1 << pin;
      return pc;
    }

    case OP_AW: {
      var pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      // TODO: pwm write somehow?
      return pc;
    }

    case OP_DR: {
      var pin;
      pc = eval(pc, &pin);
      *res = !!GET_GPIO(pin);
      return pc;
    }

    case OP_AR: {
      var pin;
      pc = eval(pc, &pin);
      // TODO: there are no analog inputs right?
      *res = 0;
      return pc;
    }

    #endif

    case OP_DEF: {
      assert(*pc >=0 && *pc <= 26);
      *res = *pc++;
      uint8_t* end = skip(pc);
      if (stubs[*res]) free(stubs[*res]);
      stubs[*res] = (uint8_t*)malloc(end - pc);
      memcpy(stubs[*res], pc, end - pc);
      return end;
    }

    case OP_RM:
      assert(*pc >=0 && *pc <= 26);
      free(stubs[*res = *pc]);
      stubs[*res] = 0;
      return pc + 1;

    case OP_RUN:
      assert(*pc >=0 && *pc <= 26);
      if (stubs[*pc]) eval(stubs[*pc], res);
      else *res = 0;
      return pc + 1;

    case OP_WAIT: {
      uint8_t* end = skip(skip(pc));
      struct event* evt = (struct event*)malloc(sizeof(*evt) + end - pc);
      memcpy(evt->code, pc, end - pc);
      evt->next = queue;
      queue = evt;
      *res = 0;
      return end;
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

int process_events() {
  struct event** parent = &queue;
  struct event* evt;

  int count = 0;
  while ((evt = *parent)) {
    var cond;
    uint8_t* prog = eval(evt->code, &cond);
    if (cond) {
      count++;
      var res;
      *parent = evt->next;
      eval(prog, &res);
      free(evt);
    }
    else {
      parent = &(evt->next);
    }
  }

  return count;
}
