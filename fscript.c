#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

typedef int64_t number;

enum opcodes {
  /* local variables */
  OP_SET = 128, OP_GET,
  /* jumps */
  OP_BACK, OP_IJUMP,
  /* stack operations */
  OP_SWAP, OP_DUP, OP_DROP,
  /* logic */
  OP_NOT, OP_AND, OP_OR, OP_XOR,
  /* bitwise logic */
  OP_BNOT, OP_BAND, OP_BOR, OP_BXOR, OP_LSHIFT, OP_RSHIFT,
  /* comparison */
  OP_EQ, OP_NEQ, OP_GTE, OP_LTE, OP_GT, OP_LT,
  /* math */
  OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_ABS,
  #ifdef OP_WIRING
  /* io */
  OP_PM, OP_DW, OP_AW, OP_DR, OP_AR,
  #endif
};

static const uint8_t* op_names = (uint8_t*)
  "SET\0GET\0"
  "BACK\0IJUMP\0"
  "SWAP\0DUP\0DROP\0"
  "NOT\0AND\0OR\0XOR\0"
  "BNOT\0BAND\0BOR\0BXOR\0LSHIFT\0RSHIFT\0"
  "EQ\0NEQ\0GTE\0LTE\0GT\0LT\0"
  "NEG\0ADD\0SUB\0MUL\0DIV\0MOD\0ABS\0"
  #ifdef OP_WIRING
  "PM\0DW\0AW\0DR\0AR\0"
  #endif
  "\0";

const uint8_t* op_to_name(uint8_t op) {
  int count = op - 128;
  const uint8_t* name = op_names;
  while (count--) while (*name++);
  return name;
}

uint8_t name_to_op(const uint8_t* name, int len) {
  uint8_t op = 128;
  const uint8_t* list = op_names;
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

char* strings;
size_t strings_size;
uint8_t strings_count;

// Convert string to numerical index
uint8_t intern(const char* string, size_t len) {
  if (!strings) {
    strings_size = len + 1;
    strings = malloc(strings_size );
    strings_count = 1;
    memcpy(strings, string, len);
    strings[len] = 0;
    return 0;
  }
  size_t offset = 0;
  uint8_t index = 0;
  while (index < strings_count) {
    int i;
    for (i = 0; strings[offset + i] == string[i];) {
      if (++i == len && !strings[offset + i]) return index;
    }
    while (strings[offset++]);
    index++;
  }
  strings = realloc(strings, strings_size + len + 1);
  memcpy(strings + strings_size, string, len);
  strings[strings_size + len] = 0;
  strings_size += len + 1;
  return strings_count++;
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
      uint8_t* start = cc++;
      while (*cc >= 'a' && *cc <= 'z') cc++;
      uint8_t index = intern((char*)start, cc - start);
      // Encode as simple integer.
      *pc++ = index;
      // printf("string found '%.*s'(%d) -> %d\n", (int)(cc - start), (char*)start, (int)(cc - start), index);
    }

    // Opcode parsing
    else if (*cc >= 'A' && *cc <= 'Z') {
      // Pre-scan the opcode to get it's length
      uint8_t *name = cc;
      do cc++; while (*cc >= 'A' && *cc <= 'Z');

      // Make sure it ended on a word boundary
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;

      // Look up the string in the table
      uint8_t op = name_to_op(name, cc - name);

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

typedef struct {
  size_t top;
  size_t len;
  number stack[];
} VM;

VM* new_vm(size_t len) {
  VM *vm = malloc(sizeof(*vm) + sizeof(number) * len);
  vm->len = len;
  vm->top = 0;
  return vm;
}

const char* push(VM* vm, number in) {
  if (vm->top >= vm->len) return "stack overflow";
  vm->stack[vm->top++] = in;
  return NULL;
}

const char* pop(VM* vm, number* out) {
  if (vm->top <= 0) return "stack underflow";
  *out = vm->stack[--vm->top];
  return NULL;
}

void dump(VM* vm) {
  printf("\nvm=%p top=%ld\n", vm, vm->top);
  for (int i = 0; i < vm->top; i++) {
    printf("%d: %"PRId64"\n", i, vm->stack[i]);
  }
  printf("\n");
}

#define check(code) { const char* e = code; if (e) return e; }

#define unop(OP, op) \
case OP: { \
  number a; \
  check(pop(vm, &a)); \
  push(vm, op a); \
  continue; \
}

#define binop(OP, op) \
case OP: { \
  number a, b; \
  check(pop(vm, &b)); \
  check(pop(vm, &a)); \
  push(vm, a op b); \
  continue; \
}

const char* eval(VM* vm, uint8_t* pc, size_t len) {
  uint8_t* end = pc + len;
  while (pc < end) {
    // dump(vm);
    // If the high bit is set, it's an opcode index.
    if (*pc & 0x80) switch ((enum opcodes)*pc++) {
    case OP_SET: {
      number i, v;
      check(pop(vm, &i));
      check(pop(vm, &v));
      if (vm->top < i) return "Stack underflow";
      vm->stack[vm->top - i] = v;
      continue;
    }
    case OP_GET: {
      if (vm->top < 1) return "Stack underflow";
      number i = vm->stack[vm->top - 1] + 1;
      if (vm->top < i) return "Stack underflow";
      vm->stack[vm->top - 1] = vm->stack[vm->top - i];
      continue;
    }
    case OP_BACK: {
      number i;
      check(pop(vm, &i));
      pc -= i;
      continue;
    }
    case OP_IJUMP: {
      number c, d;
      check(pop(vm, &d));
      check(pop(vm, &c));
      if (c) pc += d;
      continue;
    }
    case OP_SWAP: {
      if (vm->top < 2) return "Not enough items in stack to swap";
      number temp = vm->stack[vm->top - 1];
      vm->stack[vm->top - 1] = vm->stack[vm->top - 2];
      vm->stack[vm->top - 2] = temp;
      continue;
    }
    case OP_DUP: {
      if (vm->top < 1) return "Nothing on stack to dup";
      if (vm->top >= vm->len) return "No room on stack to dup";
      vm->stack[vm->top] = vm->stack[vm->top - 1];
      vm->top++;
      continue;
    }
    case OP_DROP: {
      if (vm->top < 1) return "Not enough items in stack to drop";
      vm->top--;
      continue;
    }
    unop(OP_NOT, !)
    binop(OP_AND, &&)
    binop(OP_OR, ||)
    case OP_XOR: {
      number a, b;
      check(pop(vm, &b));
      check(pop(vm, &a));
      push(vm, (a && !b) || (!a && b));
      continue;
    }
    unop(OP_BNOT, ~)
    binop(OP_BAND, &)
    binop(OP_BOR, |)
    binop(OP_BXOR, ^)
    binop(OP_LSHIFT, <<)
    binop(OP_RSHIFT, >>)
    binop(OP_EQ, =)
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
    case OP_ABS: {
      number a;
      check(pop(vm, &a));
      push(vm, a < 0 ? -a : a);
      continue;
    }
    }
    else {
      // Otherwise it's a variable length encoded integer.
      number n = *pc & 0x3f;
      if (!(*pc++ & 0x40)) {
        check(push(vm, n));
        continue;
      }
      int b = 6;
      do {
        if (pc >= end) return "Unterminated number literal";
        n |= (number)(*pc & 0x7f) << b;
        b += 7;
      } while (*pc++ & 0x80);
      check(push(vm, n));
    }
  }
  return NULL;
}

void test(const char* code, number result) {
  printf("%s => ", code);
  size_t len = strlen(code) + 1;

  uint8_t* program = malloc(len);
  memcpy(program, code, len);
  len = compile(program);
  if ((int) len < 0) {
    int offset = 1 - (int)len;
    printf("Error: Unexpected input at %d: '%s'\n", offset, code + offset);
    exit(-1);
  }
  VM* vm = new_vm(10);
  const char* e = eval(vm, program, len);
  if (e) {
    printf("Error: %s\n", e);
    dump(vm);
    exit(-1);
  }
  if (vm->top < 1) {
    printf("Error: Not enough items on stack.");
    dump(vm);
    exit(-1);
  }
  if (vm->top > 1) {
    printf("Error: Too many items on stack.");
    dump(vm);
    exit(-1);
  }
  if (vm->stack[0] != result) {
    printf("Error: Wrong result, expected %"PRId64"\n", result);
    dump(vm);
    exit(-1);
  }
  printf("%"PRId64"\n", vm->stack[0]);
}

void test_intern(const char* string, uint8_t expected) {
  uint8_t actual = intern(string, strlen(string));
  printf("%d %s\n", actual, string);
  if (actual != expected) {
    printf("Error: expected %d\n", expected);
    exit(-1);
  }
}

int main() {
  test("1000000000 0 2 GET ADD SWAP 1 SUB DUP NOT 3 IJUMP SWAP 13 BACK DROP", 500000000500000000);
  //
  //
  // test_intern("hello", 0);
  // test_intern("hello", 0);
  // test_intern("goodbye", 1);
  // test_intern("goodbye", 1);
  // test_intern("monkey", 2);
  // test_intern("giraffe", 3);
  // test_intern("hippo", 4);
  // test_intern("hello", 0);
  // test_intern("goodbye", 1);
  // test_intern("monkey", 2);
  // test_intern("giraffe", 3);
  // test_intern("a", 5);
  // test_intern("aaa", 6);
  // test_intern("aa", 7);
  // test_intern("a", 5);
  // test_intern("aaa", 6);
  // test_intern("aa", 7);
  //
  // test("1 2 ADD", 3);
  // test("1 2 SUB", -1);
  // test("1 2 SWAP SUB", 1);
  // test("1 2 ADD 3 MUL", 9);
  // test("10 3 DIV", 3);
  // test("10 3 MOD", 1);
  // test("10 NEG", -10);
  // test("10 20 SUB ABS", 10);
  // test("1", 1);
  // test("10", 10);
  // test("100", 100);
  // test("1000", 1000);
  // test("10000", 10000);
  // test("100000", 100000);
  // test("1000000", 1000000);
  // test("10000000", 10000000);
  // test("100000000", 100000000);
  // test("1000000000", 1000000000);
  // test("10000000000", 10000000000);
  // test("100000000000", 100000000000);
  // test("1000000000000", 1000000000000);
  // test("10000000000000", 10000000000000);
  // test("100000000000000", 100000000000000);
  // test("1000000000000000", 1000000000000000);
  // test("10000000000000000", 10000000000000000);
  // test("100000000000000000", 100000000000000000);
  // test("1000000000000000000", 1000000000000000000);
  // test("1000000000 NEG", -1000000000);
  // test("1 2 3 4 5 6 7 8 9 10 ADD ADD ADD ADD ADD ADD ADD ADD ADD", 55);
  // test("1 2 ADD 3 ADD 4 ADD 5 ADD 6 ADD 7 ADD 8 ADD 9 ADD 10 ADD", 55);
  // test("DEF square DUP MUL END 3 square", 9);
  return 0;
}