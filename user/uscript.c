
// Override these defines to tweak to your memory/program constraints.
#ifndef SIZE_VARS
  #define SIZE_VARS 64 // fixed int of global int sized variables
#endif
#ifndef SIZE_STACK
  #define SIZE_STACK 64 // fixed max height for stack with int sized slots
#endif
#ifndef SIZE_DATA
  #define SIZE_DATA 1024 // fixed byte size of raw global memory
#endif
#ifndef SIZE_STUBS
  #define SIZE_STUBS 64 // Fixed int of pointers for dynamic subroutines.
#endif

struct uState;

struct user_func {
  const char* name;
  // Signature for user functions
  unsigned char* (*fn)(struct uState* S, unsigned char* pc, int* res);
};

struct uState {
  void* (*malloc) (unsigned long size);
  void (*free) (void* ptr);
  int vars[SIZE_VARS];
  int stack[SIZE_STACK];
  int stack_top;
  unsigned char data[SIZE_DATA];
  unsigned char* stubs[SIZE_STUBS];
  struct user_func* funcs;
  int num_funcs;
};

enum opcodes {
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* stack manipulation */
  OP_READ, OP_WRITE, OP_INSERT, OP_REMOVE,
  /* data space manipulation */
  OP_POKE, OP_PEEK,
  /* stubs */
  OP_DEF, OP_RM, OP_RUN,
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
};
#define OP_USER_START (OP_ABS + 1)

static const char* op_names =
  "SET\0GET\0INCR\0DECR\0"
  "READ\0WRITE\0INSERT\0REMOVE\0"
  "POKE\0PEEK\0"
  "DEF\0RM\0RUN\0"
  "IF\0ELIF\0ELSE\0MATCH\0WHEN\0WHILE\0DO\0FOR\0WAIT\0"
  "NOT\0AND\0OR\0XOR\0"
  "BNOT\0BAND\0BOR\0BXOR\0LSHIFT\0RSHIFT\0"
  "EQ\0NEQ\0GTE\0LTE\0GT\0LT\0"
  "NEG\0ADD\0SUB\0MUL\0DIV\0MOD\0ABS\0"
  "\0"
;

const char* op_to_name(struct uState* S, int op) {
  if (op >= OP_USER_START && op < OP_USER_START + S->num_funcs) {
    // This range is user-defined opcodes.  Search in the provided user functions.
    return S->funcs[op - OP_USER_START].name;
  }
  if (op >= OP_SET) {
    // This range is language-defined opcodes.  Search the op_names string.
    int count = op - OP_SET;
    const char* name = op_names;
    while (count--) while (*name++);
    return name;
  }
  return 0;
}

int name_to_op(struct uState* S, const char* name, int len) {
  // First search in the language-defined names.
  int op = OP_SET;
  const char* list = op_names;
  while (*list) {
    int i;
    for (i = 0; *list == name[i]; list++) {
      if (++i == len && !*(list + 1)) return op;
    }
    while (*list++);
    op++;
  }
  // Then look in user defined names.
  int j;
  for (j = 0; j < S->num_funcs; j++) {
    const char* n = S->funcs[j].name;
    int i;
    for (i = 0; n[i] == name[i];) {
      if (++i == len && !n[i]) return j + OP_USER_START;
    }
  }
  return 0;
}

int compile(struct uState* S, unsigned char* program) {
  unsigned char *cc = program,
                *pc = program;
  while (*cc) {
    // Skip white space
    if (*cc == ' ' || *cc == '\n') cc++;

    // Integer parsing
    else if (*cc >= '0' && *cc <= '9') {

      // Parse the decimal ascii int
      int val = 0;
      do {
        val = val * 10 + *cc++ - '0';
      } while (*cc >= 0x30 && *cc < 0x40);

      // TODO: parse hex literal

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

      // Decode letters a-z as numbers 0-25
      unsigned char index = *cc++ - 'a';

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
      unsigned char *name = cc;
      do cc++; while ((*cc >= 'A' && *cc <= 'Z') ||
                      (*cc >= 'a' && *cc <= 'z') ||
                      (*cc >= '0' && *cc <= '9') ||
                       *cc == '_' || *cc == '-');

      // Make sure it ended on a word boundary
      if (*cc && *cc != ' ' && *cc != '\n') return program - cc - 1;

      // Look up the string in the table
      unsigned char op = name_to_op(S, (char*)name, cc - name);

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

unsigned char* skip(struct uState* S, unsigned char* pc) {
  if (!pc) return 0;
  // To skip user functions, pass in NULL for res.
  if (*pc >= OP_USER_START) {
    return S->funcs[*pc - OP_USER_START].fn(S, pc + 1, 0);
  }
  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {

    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // This is never valid
      return 0;

    // Need to read the length header to skip DO
    case OP_DO: {
      unsigned char count = *pc++;
      while (count--) pc = skip(S, pc);
      return pc;
    }

    // Special handling for if/elif/else chains
    case OP_IF:
      pc = skip(S, skip(S, pc)); // cond/body
      while (*pc == OP_ELIF) pc = skip(S, skip(S, pc + 1)); // elif/cond/body
      if (*pc == OP_ELSE) pc = skip(S, pc + 1); // else/body
      return pc;

    // Special handling for match/when/else chains
    case OP_MATCH:
      pc = skip(S, pc); // value
      while (*pc == OP_WHEN) pc = skip(S, skip(S, pc + 1)); // when/val/body
      if (*pc == OP_ELSE) pc = skip(S, pc + 1); // else/body
      return pc;

    // Opcodes that consume one expression
    case OP_NOT: case OP_BNOT: case OP_NEG: case OP_ABS:
    case OP_PEEK:
    case OP_WAIT:
    case OP_GET: case OP_RM: case OP_RUN: case OP_READ: case OP_REMOVE:
      return skip(S, pc);

    // Opcodes that consume one two expressions
    case OP_SET: case OP_INCR: case OP_DECR: case OP_DEF: case OP_WRITE: case OP_INSERT:
    case OP_WHILE:
    case OP_POKE:
    case OP_AND: case OP_OR: case OP_XOR:
    case OP_BAND: case OP_BOR: case OP_BXOR: case OP_LSHIFT: case OP_RSHIFT:
    case OP_EQ: case OP_NEQ: case OP_GTE: case OP_LTE: case OP_GT: case OP_LT:
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
      return skip(S, skip(S, pc));

    // Opcodes that consume four expressions
    case OP_FOR:
      return skip(S, skip(S, skip(S, skip(S, pc))));

  }

  // Otherwise it's a variable length encoded integer.
  if (!(*pc++ & 0x40)) return pc;
  while (*pc++ & 0x80);
  return pc;
}

#define binop(code, op) \
  case code: { \
    int a, b; \
    pc = eval(S, pc, &a); \
    pc = eval(S, pc, &b); \
    *res = a op b; \
    return pc; \
  }

#define unop(code, op) \
  case code: { \
    pc = eval(S, pc, res); \
    *res = op*res; \
    return pc; \
  }

unsigned char* eval(struct uState* S, unsigned char* pc, int* res) {
  if (!pc) return 0;

  // Route user functions to their handlers.
  if (*pc >= OP_USER_START) {
    return S->funcs[*pc - OP_USER_START].fn(S, pc + 1, res);
  }

  // If the high bit is set, it's an opcode index.
  if (*pc & 0x80) switch ((enum opcodes)*pc++) {
    case OP_ELIF: case OP_WHEN: case OP_ELSE:
      // Unexpected elif, when, or else opcode
      return 0;

    case OP_SET: {
      int idx;
      pc = eval(S, pc, &idx);
      pc = eval(S, pc, res);
      S->vars[idx] = *res;
      return pc;
    }
    case OP_GET: {
      int idx;
      pc = eval(S, pc, &idx);
      *res = S->vars[idx];
      return pc;
    }

    case OP_INCR: {
      int idx, step;
      pc = eval(S, pc, &idx);
      pc = eval(S, pc, &step);
      *res = S->vars[idx] += step;
      return pc;
    }

    case OP_DECR: {
      int idx, step;
      pc = eval(S, pc, &idx);
      pc = eval(S, pc, &step);
      *res = S->vars[idx] -= step;
      return pc;
    }

    case OP_READ: {
      int idx;
      pc = eval(S, pc, &idx);
      *res = S->stack[S->stack_top - idx - 1];
      return pc;
    }

    case OP_WRITE: {
      int idx;
      pc = eval(S, pc, &idx);
      pc = eval(S, pc, res);
      S->stack[S->stack_top - idx - 1] = *res;
      return pc;
    }

    case OP_REMOVE: {
      int idx;
      pc = eval(S, pc, &idx);
      *res = S->stack[S->stack_top - idx - 1];
      while (idx--) {
        S->stack[S->stack_top - idx] = S->stack[S->stack_top - idx + 1];
      }
      S->stack_top--;
      return pc;
    }
    case OP_INSERT: {
      int idx;
      pc = eval(S, pc, &idx);
      unsigned char base = S->stack_top - idx;
      pc = eval(S, pc, res);
      while (idx--) {
        S->stack[base + idx + 1] = S->stack[base + idx];
      }
      S->stack[base] = *res;
      S->stack_top++;
      return pc;
    }

    case OP_PEEK:
      pc = eval(S, pc, res);
      if (*res >= 0 && *res < SIZE_DATA) {
        *res = S->data[*res];
      }
      return pc;

    case OP_POKE: {
      int offset;
      pc = eval(S, pc, &offset);
      pc = eval(S, pc, res);
      if (offset >= 0 && offset < SIZE_DATA) {
        S->data[offset] = *res;
      }
      return pc;
    }

    case OP_IF: {
      int cond;
      char done = 0;
      *res = 0;
      pc = eval(S, pc, &cond);
      if (cond) {
        done = 1;
        pc = eval(S, pc, res);
      }
      else pc = skip(S, pc);
      while (*pc == OP_ELIF) {
        pc++;
        if (done) {
          pc = skip(S, skip(S, pc));
        }
        else {
          pc = eval(S, pc, &cond);
          if (cond) {
            done = 1;
            pc = eval(S, pc, res);
          }
          else pc = skip(S, pc);
        }
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(S, pc);
        else pc = eval(S, pc, res);
      }
      return pc;
    }

    case OP_MATCH: {
      int val, cond;
      char done = 0;
      *res = 0;
      pc = eval(S, pc, &val);
      while (*pc == OP_WHEN) {
        pc++;
        if (done) {
          pc = skip(S, pc); // cond
          pc = skip(S, pc); // val
        }
        pc = eval(S, pc, &cond);
        if (cond == val) {
          done = 1;
          pc = eval(S, pc, res);
        }
        else pc = skip(S, pc);
      }
      if (*pc == OP_ELSE) {
        pc++;
        if (done) pc = skip(S, pc);
        else pc = eval(S, pc, res);
      }
      return pc;
    }

    case OP_WHILE: {
      unsigned char* c = pc;
      int cond;
      *res = 0;
      while (pc = eval(S, c, &cond), cond) {
        eval(S, pc, res);
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
      }
      return skip(S, pc);
    }

    case OP_DO: {
      unsigned char count = *pc++;
      *res = 0;
      while (count--) pc = eval(S, pc, res);
      return pc;
    }

    case OP_FOR: {
      int idx;
      pc = eval(S, pc, &idx);
      int start, end;
      pc = eval(S, pc, &start);
      pc = eval(S, pc, &end);
      unsigned char* body = pc;
      *res = 0;
      while (start <= end) {
        S->vars[idx] = start++;
        pc = eval(S, body, res);
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
      }
      return pc;
    }

    unop(OP_NOT, !)
    case OP_AND:
      pc = eval(S, pc, res);
      if (*res) pc = eval(S, pc, res);
      else pc = skip(S, pc);
      return pc;
    case OP_OR:
      pc = eval(S, pc, res);
      if (*res) pc = skip(S, pc);
      else pc = eval(S, pc, res);
      return pc;
    case OP_XOR: {
      int a, b;
      pc = eval(S, pc, &a);
      pc = eval(S, pc, &b);
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
      pc = eval(S, pc, res);
      if (*res < 0) *res = -*res;
      return pc;

    case OP_DEF: {
      pc = eval(S, pc, res);
      unsigned char* end = skip(S, pc);
      if (*res >= 0 && *res < SIZE_STUBS) {
        if (S->stubs[*res]) S->free(S->stubs[*res]);
        int len = end - pc;
        S->stubs[*res] = S->malloc(len);
        int i;
        for (i = 0; i < len; i++) {
          S->stubs[*res][i] = pc[i];
        }
      }
      return end;
    }

    case OP_RM:
      pc = eval(S, pc, res);
      if (*res >= 0 && *res < SIZE_STUBS) {
        S->free(S->stubs[*res]);
        S->stubs[*res] = 0;
      }
      return pc;

    case OP_RUN:
      pc = eval(S, pc, res);
      if (*res >= 0 && *res < SIZE_STUBS && S->stubs[*res]) {
        eval(S, S->stubs[*res], res);
      }
      else {
        *res = 0;
      }
      return pc;

    case OP_WAIT: {
      unsigned char* start = pc;
      while (pc = eval(S, start, res), !*res) {
        #ifdef CHECKER
          if (CHECKER) break;
        #endif
      }
      return pc;
    }

  }

  // Otherwise it's a variable length encoded integer.
  *res = *pc & 0x3f;
  if (!(*pc++ & 0x40)) return pc;
  int b = 6;
  do {
    *res |= (int)(*pc & 0x7f) << b;
    b += 7;
  } while (*pc++ & 0x80);
  return pc;
}
