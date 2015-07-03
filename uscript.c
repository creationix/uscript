#include <stdint.h>
#include <string.h>

// #include "rpi-io.c"

#if defined(SPARK)
  #include "application.h"
  #define number int32_t
  #define OP_WIRING
  #define assert(x)
#elif defined(ARDUINO)
  #include "Arduino.h"
  #define number int32_t
  #define OP_WIRING
  #define assert(x)
#else
  #include <assert.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <inttypes.h>
  #define number int64_t
  #ifdef BCM2708_PERI_BASE
    #define OP_WIRING
  #endif
#endif

typedef void (*write_string_fn)(const char* str);
typedef void (*write_char_fn)(char c);
typedef void (*write_number_fn)(number num);

struct state {
  write_string_fn write_string;
  write_char_fn write_char;
  write_number_fn write_number;
  number vars[26];
  uint8_t* stubs[26];
};

enum opcodes {
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* control flow */
  OP_IF, OP_ELIF, OP_ELSE, OP_MATCH, OP_WHEN, OP_WHILE, OP_DO, OP_FOR, OP_WAIT,
  /* logic (short circuit and/or) */
  OP_NOT, OP_AND, OP_OR, OP_XOR,
  /* bitwise logic */
  OP_BNOT, OP_BAND, OP_BOR, OP_BXOR, OP_LSHIFT, OP_RSHIFT,
  /* comparison */
  OP_EQ, OP_NEQ, OP_GTE, OP_LTE, OP_GT, OP_LT,
  /* math */
  OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_ABS,
  /* misc */
  OP_DELAY, OP_RAND, OP_PRINT,
  /* stubs */
  OP_DEF, OP_RM, OP_RUN
  #ifdef OP_WIRING
  /* io */
  ,OP_PM, OP_DW, OP_AW, OP_DR, OP_AR
  #endif
};

static const char* op_names =
  "SET\0GET\0INCR\0DECR\0"
  "IF\0ELIF\0ELSE\0MATCH\0WHEN\0WHILE\0DO\0FOR\0WAIT\0"
  "NOT\0AND\0OR\0XOR\0"
  "BNOT\0BAND\0BOR\0BXOR\0LSHIFT\0RSHIFT\0"
  "EQ\0NEQ\0GTE\0LTE\0GT\0LT\0"
  "NEG\0ADD\0SUB\0MUL\0DIV\0MOD\0ABS\0"
  "DELAY\0RAND\0PRINT\0"
  "DEF\0RM\0RUN\0"
  #ifdef OP_WIRING
  "PM\0DW\0AW\0DR\0AR\0"
  #endif
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

int compile(uint8_t* program) {
  uint8_t *cc = program,
          *pc = program;
  while (*cc) {
    // Skip white space
    if (*cc == ' ' || *cc == '\n') cc++;

    // Integer parsing
    else if (*cc >= '0' && *cc <= '9') {

      // Parse the decimal ascii number
      number val = 0;
      do {
        val = val * 10 + *cc++ - '0';
      } while (*cc >= 0x30 && *cc < 0x40);

      // Make sure it ended on a word boundary
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;

      // printf("INTEGER %"PRId64"\n", val);

      // Encode as a variable length binary integer.
      if (val < 0x40) {
        *pc++ = val & 0x3f;
      }
      else {
        *pc++ = (val & 0x3f) | 0x40;
        val >>= 6;
        while (val >= 0x80) {
          *pc++ = (val & 0x7f) | 0x80;
          val >>= 7;
        }
        *pc++ = val & 0x7f;
      }

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

    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // This is never valid
      return NULL;

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

    case OP_IF:
      pc = skip(skip(pc)); // cond/body
      while (*pc == OP_ELIF) pc = skip(skip(pc + 1)); // elif/cond/body
      if (*pc == OP_ELSE) pc = skip(pc + 1); // else/body
      return pc;

    case OP_MATCH:
      pc = skip(pc); // value
      while (*pc == OP_WHEN) pc = skip(skip(pc + 1)); // when/val/body
      if (*pc == OP_ELSE) pc = skip(pc + 1); // else/body
      return pc;

    // Opcodes that consume one expression
    case OP_NOT: case OP_BNOT: case OP_NEG: case OP_ABS:
    case OP_DELAY: case OP_RAND: case OP_PRINT:
    #ifdef OP_WIRING
    case OP_DR: case OP_AR:
    #endif
    case OP_WAIT:
      return skip(pc);

    // Opcodes that consume two expressions
    case OP_WHILE:
    case OP_AND: case OP_OR: case OP_XOR:
    case OP_BAND: case OP_BOR: case OP_BXOR: case OP_LSHIFT: case OP_RSHIFT:
    case OP_EQ: case OP_NEQ: case OP_GTE: case OP_LTE: case OP_GT: case OP_LT:
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
    #ifdef OP_WIRING
    case OP_PM: case OP_DW: case OP_AW:
    #endif
      return skip(skip(pc));


  }

  // Otherwise it's a variable length encoded integer.
  if (!(*pc++ & 0x40)) return pc;
  while (*pc++ & 0x80);
  return pc;
}

#define binop(code, op) \
  case code: { \
    number a, b; \
    pc = eval(vm, pc, &a); \
    pc = eval(vm, pc, &b); \
    *res = a op b; \
    return pc; \
  }

#define unop(code, op) \
  case code: { \
    pc = eval(vm, pc, res); \
    *res = op*res; \
    return pc; \
  }

uint8_t* eval(struct state* vm, uint8_t* pc, number* res) {

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      return 0;

    case OP_SET: {
      uint8_t idx = *pc++;
      pc = eval(vm, pc, res);
      vm->vars[idx] = *res;
      return pc;
    }
    case OP_GET:
      *res = vm->vars[*pc];
      return pc + 1;

    case OP_INCR: {
      number step;
      uint8_t idx = *pc++;
      pc = eval(vm, pc, &step);
      *res = vm->vars[idx] += step;
      return pc;
    }

    case OP_DECR: {
      number step;
      uint8_t idx = *pc++;
      pc = eval(vm, pc, &step);
      *res = vm->vars[idx] -= step;
      return pc;
    }

    case OP_IF: {
      number cond;
      char done = 0;
      *res = 0;
      pc = eval(vm, pc, &cond);
      if (cond) {
        done = 1;
        pc = eval(vm, pc, res);
      }
      else pc = skip(pc);
      while (*pc == OP_ELIF) {
        pc++;
        if (done) {
          pc = skip(skip(pc));
        }
        else {
          pc = eval(vm, pc, &cond);
          if (cond) {
            done = 1;
            pc = eval(vm, pc, res);
          }
          else pc = skip(pc);
        }
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(pc);
        else pc = eval(vm, pc, res);
      }
      return pc;
    }

    case OP_MATCH: {
      number val, cond;
      char done = 0;
      *res = 0;
      pc = eval(vm, pc, &val);
      while (*pc == OP_WHEN) {
        pc++;
        if (done) {
          pc = skip(pc); // cond
          pc = skip(pc); // val
        }
        pc = eval(vm, pc, &cond);
        if (cond == val) {
          done = 1;
          pc = eval(vm, pc, res);
        }
        else pc = skip(pc);
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(pc);
        else pc = eval(vm, pc, res);
      }
      return pc;
    }

    case OP_WHILE: {
      uint8_t* c = pc;
      number cond;
      *res = 0;
      while (pc = eval(vm, c, &cond), cond) {
        eval(vm, pc, res);
      }
      return skip(pc);
    }

    case OP_DO: {
      uint8_t count = *pc++;
      *res = 0;
      while (count--) pc = eval(vm, pc, res);
      return pc;
    }

    case OP_FOR: {
      uint8_t idx = *pc++;
      number start, end;
      pc = eval(vm, pc, &start);
      pc = eval(vm, pc, &end);
      uint8_t* body = pc;
      *res = 0;
      while (start <= end) {
        vm->vars[idx] = start++;
        pc = eval(vm, body, res);
      }
      return pc;
    }

    unop(OP_NOT, !)
    case OP_AND:
      pc = eval(vm, pc, res);
      if (*res) pc = eval(vm, pc, res);
      else pc = skip(pc);
      return pc;
    case OP_OR:
      pc = eval(vm, pc, res);
      if (*res) pc = skip(pc);
      else pc = eval(vm, pc, res);
      return pc;
    case OP_XOR: {
      number a, b;
      pc = eval(vm, pc, &a);
      pc = eval(vm, pc, &b);
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

    case OP_ABS:
      pc = eval(vm, pc, res);
      if (*res < 0) *res = -*res;
      return pc;

    case OP_PRINT:
      pc = eval(vm, pc, res);
      vm->write_number(*res);
      vm->write_string("\r\n");
      return pc;

    case OP_RAND:
      pc = eval(vm, pc, res);
      *res = deadbeef_rand() % *res;
      return pc;

    #ifndef ARDUINO

    case OP_DELAY: {
      pc = eval(vm, pc, res);
      struct timeval t;
      t.tv_sec = *res / 1000;
      t.tv_usec = (*res % 1000) * 1000;
      select(0, NULL, NULL, NULL, &t);
      return pc;
    }

    #else

    case OP_DELAY:
      pc = eval(vm, pc, res);
      delay(*res);
      return pc;

    case OP_PM: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      pinMode(pin, *res);
      return pc;
    }

    case OP_DW: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      digitalWrite(pin, *res);
      return pc;
    }

    case OP_AW: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      analogWrite(pin, *res);
      return pc;
    }

    case OP_DR: {
      number pin;
      pc = eval(vm, pc, &pin);
      *res = digitalRead(pin);
      return pc;
    }

    case OP_AR: {
      number pin;
      pc = eval(vm, pc, &pin);
      *res = analogRead(pin);
      return pc;
    }

    #endif
    #ifdef BCM2708_PERI_BASE

    case OP_PM: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      INP_GPIO(pin);
      if (*res) OUT_GPIO(pin);
      return pc;
    }

    case OP_DW: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      if (*res) GPIO_SET = 1 << pin;
      else GPIO_CLR = 1 << pin;
      return pc;
    }

    case OP_AW: {
      number pin;
      pc = eval(vm, pc, &pin);
      pc = eval(vm, pc, res);
      // TODO: pwm write somehow?
      return pc;
    }

    case OP_DR: {
      number pin;
      pc = eval(vm, pc, &pin);
      *res = !!GET_GPIO(pin);
      return pc;
    }

    case OP_AR: {
      number pin;
      pc = eval(vm, pc, &pin);
      // TODO: there are no analog inputs right?
      *res = 0;
      return pc;
    }

    #endif

    case OP_DEF: {
      assert(*pc >=0 && *pc <= 26);
      *res = *pc++;
      uint8_t* end = skip(pc);
      if (vm->stubs[*res]) free(vm->stubs[*res]);
      vm->stubs[*res] = (uint8_t*)malloc(end - pc);
      memcpy(vm->stubs[*res], pc, end - pc);
      return end;
    }

    case OP_RM:
      assert(*pc >=0 && *pc <= 26);
      free(vm->stubs[*res = *pc]);
      vm->stubs[*res] = 0;
      return pc + 1;

    case OP_RUN:
      assert(*pc >=0 && *pc <= 26);
      if (vm->stubs[*pc]) eval(vm, vm->stubs[*pc], res);
      else *res = 0;
      return pc + 1;

    case OP_WAIT: {
      uint8_t* start = pc;
      while (pc = eval(vm, start, res), !*res);
      return pc;
    }

  }

  // Otherwise it's a variable length encoded integer.
  *res = *pc & 0x3f;
  if (!(*pc++ & 0x40)) return pc;
  int b = 6;
  do {
    *res |= (number)(*pc & 0x7f) << b;
    b += 7;
  } while (*pc++ & 0x80);
  return pc;
}

#define MAX_LEN 4096
uint8_t line[MAX_LEN];
int offset;

void handle_input(struct state* vm, char c) {
  if (offset < MAX_LEN && c >= 0x20 && c < 0x7f) {
    line[offset++] = c;
    vm->write_char(c);
  }
  else if (offset > 0 && (c == 127 || c == 8)) {
    line[--offset] = 0;
    vm->write_string("\x08 \x08");
  }
  else if (c == '\r' || c == '\n') {
    vm->write_string("\r\n");
    if (offset) {
      line[offset++] = 0;
      int len = compile(line);
      if ((int) len < 0) {
        int offset = 1 - (int)len;
        while (offset--) vm->write_string(" ");
        vm->write_string("^\r\nUnexpected input\r\n");
      }
      else {
        int offset = 0;
        while (offset < len) {
          number result;
          offset = eval(vm, line + offset, &result) - line;
          vm->write_number(result);
          vm->write_string("\r\n");
        }
      }

    }
    offset = 0;
    vm->write_string("> ");
  }
}

void start_state(struct state* vm) {
  vm->write_string("Welcome to uscript.\r\n> ");
}
