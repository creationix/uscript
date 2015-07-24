#include <stdint.h>
#include <string.h>

#define NUM_VARS 64
#define NUM_STUBS 64
#define NUM_ARRAYS 64
#define STACK_SIZE 64
#define REPL_BUFFER 4096
#define EEPROM_SIZE 4096

#if defined(SPARK)
  #include "application.h"
  typedef int32_t number;
  #define OP_WIRING
  #define assert(x)
#elif defined(ARDUINO)
  #if ARDUINO >= 100
    #include "Arduino.h"
  #endif
  #include "NeoPixelesp8266.c"
  typedef int32_t number;
  #define OP_WIRING
  #define CHECKER yield(),!digitalRead(0)
  #define assert(x)
#else
  #include <assert.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <inttypes.h>
  typedef int64_t number;
  #ifdef BCM2708_PERI_BASE
    #define OP_WIRING
  #endif
#endif

typedef void (*write_string_fn)(const char* str);
typedef void (*write_char_fn)(char c);
typedef void (*write_number_fn)(number num);
typedef int (*idle_fn)();

write_string_fn write_string;
write_char_fn write_char;
write_number_fn write_number;
idle_fn idle;
number vars[NUM_VARS];
number stack[STACK_SIZE];
int stack_top;
uint8_t* stubs[NUM_STUBS];
number* arrays[NUM_ARRAYS];

enum opcodes {
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* stack manipulation */
  OP_READ, OP_WRITE, OP_INSERT, OP_REMOVE,
  /* array manipulation */
  OP_RESIZE, OP_POKE, OP_PEEK,
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
  OP_DEF, OP_RM, OP_RUN, OP_LIST,
  #ifdef ARDUINO
  OP_SAVE,
  #endif
  #ifdef OP_WIRING
  /* io */
  OP_PM, OP_DW, OP_AW, OP_DR, OP_AR,
  OP_TONE, OP_NEOPIX,
  #endif
};

static const char* op_names =
  "SET\0GET\0INCR\0DECR\0"
  "READ\0WRITE\0INSERT\0REMOVE\0"
  "RESIZE\0POKE\0PEEK\0"
  "IF\0ELIF\0ELSE\0MATCH\0WHEN\0WHILE\0DO\0FOR\0WAIT\0"
  "NOT\0AND\0OR\0XOR\0"
  "BNOT\0BAND\0BOR\0BXOR\0LSHIFT\0RSHIFT\0"
  "EQ\0NEQ\0GTE\0LTE\0GT\0LT\0"
  "NEG\0ADD\0SUB\0MUL\0DIV\0MOD\0ABS\0"
  "DELAY\0RAND\0PRINT\0"
  "DEF\0RM\0RUN\0LIST\0"
  #ifdef OP_WIRING
  "SAVE\0PM\0DW\0AW\0DR\0AR\0TONE\0NEOPIX\0"
  #endif
  "\0"
;

const char* op_to_name(int op) {
  int count = op - 128;
  const char* name = op_names;
  while (count--) while (*name++);
  return name;
}

int name_to_op(const char* name, int len) {
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

void dump(uint8_t* pc, int len) {
  uint8_t* end = pc + len;
  while (pc < end) {
    write_char(' ');
    // If the high bit is set, it's an opcode index.
    if (*pc & 0x80) {
      write_string(op_to_name(*pc++));
      continue;
    }
    // Otherwise it's a variable length encoded integer.
    number val = *pc & 0x3f;
    if (*pc++ & 0x40) {
      int b = 6;
      do {
        val |= (number)(*pc & 0x7f) << b;
        b += 7;
      } while (*pc++ & 0x80);
    }
    write_number(val);
  }
}

uint8_t* skip(uint8_t* pc) {
  if (!pc) return NULL;
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

    // Special handling for if/elif/else chains
    case OP_IF:
      pc = skip(skip(pc)); // cond/body
      while (*pc == OP_ELIF) pc = skip(skip(pc + 1)); // elif/cond/body
      if (*pc == OP_ELSE) pc = skip(pc + 1); // else/body
      return pc;

    // Special handling for match/when/else chains
    case OP_MATCH:
      pc = skip(pc); // value
      while (*pc == OP_WHEN) pc = skip(skip(pc + 1)); // when/val/body
      if (*pc == OP_ELSE) pc = skip(pc + 1); // else/body
      return pc;

    // Opcodes with no arguments
    #ifdef ARDUINO
    case OP_SAVE:
    #endif
    case OP_LIST: return pc;

    // Opcodes that consume one expression
    case OP_NOT: case OP_BNOT: case OP_NEG: case OP_ABS:
    case OP_DELAY: case OP_RAND: case OP_PRINT:
    #ifdef OP_WIRING
    case OP_DR: case OP_AR:
    #endif
    case OP_WAIT:
    case OP_GET: case OP_RM: case OP_RUN: case OP_READ: case OP_REMOVE:
      return skip(pc);

    // Opcodes that consume one two expressions
    case OP_RESIZE: case OP_PEEK:
    case OP_SET: case OP_INCR: case OP_DECR: case OP_DEF: case OP_WRITE: case OP_INSERT:
    case OP_WHILE:
    case OP_AND: case OP_OR: case OP_XOR:
    case OP_BAND: case OP_BOR: case OP_BXOR: case OP_LSHIFT: case OP_RSHIFT:
    case OP_EQ: case OP_NEQ: case OP_GTE: case OP_LTE: case OP_GT: case OP_LT:
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
    #ifdef OP_WIRING
    case OP_PM: case OP_DW: case OP_AW:
    #endif
      return skip(skip(pc));

    // Opcodes that consume three expressions
    case OP_POKE:
    #ifdef OP_WIRING
    case OP_TONE:
    case OP_NEOPIX:
    #endif
      return skip(skip(skip(pc)));

    // Opcodes that consume four expressions
    case OP_FOR:
      return skip(skip(skip(skip(pc))));

  }

  // Otherwise it's a variable length encoded integer.
  if (!(*pc++ & 0x40)) return pc;
  while (*pc++ & 0x80);
  return pc;
}

#define binop(code, op) \
  case code: { \
    number a, b; \
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

uint8_t* eval(uint8_t* pc, number* res) {
  if (!pc) return NULL;

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      return 0;

    case OP_SET: {
      number idx;
      pc = eval(pc, &idx);
      pc = eval(pc, res);
      vars[idx] = *res;
      return pc;
    }
    case OP_GET: {
      number idx;
      pc = eval(pc, &idx);
      *res = vars[idx];
      return pc;
    }

    case OP_INCR: {
      number idx, step;
      pc = eval(pc, &idx);
      pc = eval(pc, &step);
      *res = vars[idx] += step;
      return pc;
    }

    case OP_DECR: {
      number idx, step;
      pc = eval(pc, &idx);
      pc = eval(pc, &step);
      *res = vars[idx] -= step;
      return pc;
    }

    case OP_READ: {
      number idx;
      pc = eval(pc, &idx);
      *res = stack[stack_top - idx - 1];
      return pc;
    }

    case OP_WRITE: {
      number idx;
      pc = eval(pc, &idx);
      pc = eval(pc, res);
      stack[stack_top - idx - 1] = *res;
      return pc;
    }

    case OP_REMOVE: {
      number idx;
      pc = eval(pc, &idx);
      *res = stack[stack_top - idx - 1];
      while (idx--) {
        stack[stack_top - idx] = stack[stack_top - idx + 1];
      }
      stack_top--;
      return pc;
    }
    case OP_INSERT: {
      number idx;
      pc = eval(pc, &idx);
      uint8_t base = stack_top - idx;
      pc = eval(pc, res);
      while (idx--) {
        stack[base + idx + 1] = stack[base + idx];
      }
      stack[base] = *res;
      stack_top++;
      return pc;
    }

    case OP_RESIZE: {
      number idx;
      pc = eval(pc, &idx);
      pc = eval(pc, res);
      if (arrays[idx]) free(arrays[idx]);
      if (*res) arrays[idx] = (number*)malloc(sizeof(number) * *res);
      if (!arrays[idx]) *res = 0;
      return pc;
    }

    case OP_PEEK: {
      number idx;
      pc = eval(pc, &idx);
      pc = eval(pc, res);
      *res = arrays[idx][*res];
      return pc;
    }

    case OP_POKE: {
      number idx;
      pc = eval(pc, &idx);
      number offset;
      pc = eval(pc, &offset);
      pc = eval(pc, res);
      arrays[idx][offset] = *res;
      return pc;
    }

    case OP_IF: {
      number cond;
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
      number val, cond;
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
      number cond;
      *res = 0;
      while (pc = eval(c, &cond), cond) {
        eval(pc, res);
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
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
      number idx;
      pc = eval(pc, &idx);
      number start, end;
      pc = eval(pc, &start);
      pc = eval(pc, &end);
      uint8_t* body = pc;
      *res = 0;
      while (start <= end) {
        vars[idx] = start++;
        pc = eval(body, res);
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
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
      number a, b;
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

    case OP_ABS:
      pc = eval(pc, res);
      if (*res < 0) *res = -*res;
      return pc;

    case OP_PRINT:
      pc = eval(pc, res);
      write_number(*res);
      write_string("\r\n");
      return pc;

    case OP_RAND:
      pc = eval(pc, res);
      *res = deadbeef_rand() % *res;
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

    #else

    case OP_DELAY:
      pc = eval(pc, res);
      delay(*res);
      return pc;

    case OP_PM: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      pinMode(pin, *res);
      return pc;
    }

    case OP_DW: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      digitalWrite(pin, *res);
      return pc;
    }

    case OP_AW: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      analogWrite(pin, *res);
      return pc;
    }

    case OP_DR: {
      number pin;
      pc = eval(pc, &pin);
      *res = digitalRead(pin);
      return pc;
    }

    case OP_AR: {
      number pin;
      pc = eval(pc, &pin);
      *res = analogRead(pin);
      return pc;
    }

    case OP_TONE: {
      number pin, dur;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      pc = eval(pc, &dur);
      int p = 1000000 / *res;
      int d = p >> 1;
      dur *= 1000;
      while ((dur -= p) > 0) {
        digitalWrite(pin, 1);
        delayMicroseconds(d);
        digitalWrite(pin, 0);
        delayMicroseconds(d);
      }
      return pc;
    }

    case OP_NEOPIX: {
      number pin, slot, len;
      pc = eval(pc, &pin);
      pc = eval(pc, &slot);
      pc = eval(pc, &len);
      *res = len;

      uint8_t pixels[3 * 64];
      int j = 0;
      for (int i = 0; i < len; i++) {
        uint32_t pixel = arrays[slot][i];
        pixels[j++] = (pixel >> 8) & 0xff;
        pixels[j++] = (pixel >> 16) & 0xff;
        pixels[j++] = pixel & 0xff;
      }
      send_pixels_800(pixels, pixels + j, pin);

      return pc;
    }

    #endif
    #ifdef BCM2708_PERI_BASE

    case OP_PM: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      INP_GPIO(pin);
      if (*res) OUT_GPIO(pin);
      return pc;
    }

    case OP_DW: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      if (*res) GPIO_SET = 1 << pin;
      else GPIO_CLR = 1 << pin;
      return pc;
    }

    case OP_AW: {
      number pin;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      // TODO: pwm write somehow?
      return pc;
    }

    case OP_DR: {
      number pin;
      pc = eval(pc, &pin);
      *res = !!GET_GPIO(pin);
      return pc;
    }

    case OP_AR: {
      number pin;
      pc = eval(pc, &pin);
      // TODO: there are no analog inputs right?
      *res = 0;
      return pc;
    }

    case OP_TONE: {
      number pin, dur;
      pc = eval(pc, &pin);
      pc = eval(pc, res);
      pc = eval(pc, &dur);

      struct timeval t;
      t.tv_sec = 0;

      int p = 1000000 / *res;
      dur *= 1000;
      t.tv_usec = p >> 1;
      while ((dur -= p) > 0) {
        GPIO_SET = 1 << pin;
        select(0, NULL, NULL, NULL, &t);
        GPIO_CLR = 1 << pin;
        select(0, NULL, NULL, NULL, &t);
      }
      return pc;
    }

    #endif

    case OP_DEF: {
      pc = eval(pc, res);
      assert(*res >=0 && *res <= 26);
      uint8_t* end = skip(pc);
      if (stubs[*res]) free(stubs[*res]);
      stubs[*res] = (uint8_t*)malloc(end - pc);
      memcpy(stubs[*res], pc, end - pc);
      return end;
    }

    case OP_RM:
      pc = eval(pc, res);
      assert(*res >=0 && *res <= 26);
      free(stubs[*res]);
      stubs[*res] = 0;
      return pc;

    case OP_RUN:
      pc = eval(pc, res);
      assert(*res >=0 && *res <= 26);
      if (stubs[*res]) eval(stubs[*res], res);
      else *res = 0;
      return pc;

    case OP_WAIT: {
      uint8_t* start = pc;
      while (pc = eval(start, res), !*res) {
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
      }
      return pc;
    }


    #ifdef ARDUINO
    case OP_SAVE: {
      int i;
      *res = 0;
      int o = 0;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.write(o++, 'u');
      for (i = 0; i < NUM_STUBS; i++) {
        if (!stubs[i]) continue;
        write_string("Saving ");
        write_char(i + 'a');
        write_string("...\r\n");
        EEPROM.write(o++, i);
        int len = skip(stubs[i]) - stubs[i];
        EEPROM.write(o++, len >> 8);
        EEPROM.write(o++, len & 0xff);
        int j;
        for (j = 0; j < len; j++) {
          EEPROM.write(o++, stubs[i][j]);
        }
      }
      EEPROM.write(o++, 'u');
      EEPROM.end();
      *res = o;
      return pc;
    }

    #endif

    case OP_LIST: {
      int i;
      *res = 2;
      for (i = 0; i < NUM_STUBS; i++) {
        if (!stubs[i]) continue;
        write_string("DEF ");
        write_char(i + 'a');
        int len = skip(stubs[i]) - stubs[i];
        dump(stubs[i], len);
        write_string("\r\n");
        *res += len + 3;
      }
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

uint8_t line[REPL_BUFFER];
int offset;

void handle_input(char c) {
  if (offset < REPL_BUFFER && c >= 0x20 && c < 0x7f) {
    line[offset++] = c;
    write_char(c);
  }
  else if (offset > 0 && (c == 127 || c == 8)) {
    line[--offset] = 0;
    write_string("\x08 \x08");
  }
  else if (c == '\r' || c == '\n') {
    write_string("\r\n");
    if (offset) {
      line[offset++] = 0;
      int len = compile(line);
      if ((int) len < 0) {
        int offset = 1 - (int)len;
        while (offset--) write_string(" ");
        write_string("^\r\nUnexpected input\r\n");
      }
      else {
        int offset = 0;
        while (offset < len) {
          number result;
          offset = eval(line + offset, &result) - line;
          write_number(result);
          write_string("\r\n");
        }
      }

    }
    offset = 0;
    write_string("> ");
  }
}

void start() {
  write_string("\r\nWelcome to uscript.\r\n");
  #ifdef ARDUINO
    pinMode(0, 0);
    EEPROM.begin(EEPROM_SIZE);
    int o = 0;
    if (EEPROM.read(o++) == 'u') {
      while (EEPROM.read(o) != 'u') {
        int key = EEPROM.read(o++);
        write_string("Loading ");
        write_char(key + 'a');
        write_string("...\r\n");
        int len = EEPROM.read(o++) << 8;
        len |= EEPROM.read(o++);
        uint8_t* stub = (uint8_t*)malloc(len);
        stubs[key] = stub;
        int j;
        for (j = 0; j < len; j++) {
          stub[j] = EEPROM.read(o++);
        }
      }
    }
    EEPROM.end();
  #endif
  if (stubs[0]) {
    write_string("Running auto script...\r\n");
    number out;
    eval(stubs[0], &out);
  }
  write_string("> ");
}
