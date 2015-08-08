#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
  Empty = 128,
  Add, Sub, Mul, Div, Mod, Neg,
  Do, Doing, End,
  And, Or,
  If, Then, ElIf, Else,
  Def, Call,
} instruction;

const char* names[] = {
  "Empty",
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Do", "Doing", "End",
  "And", "Or",
  "If", "Then", "ElIf", "Else",
  "Def", "Call",
  0
};

#define LAST_OP Def
#define DEF(id, nargs) Def, id, nargs
#define CALL(id) (255 - id)

#define MAX_ISTACK 512
#define MAX_VSTACK 128
#define MAX_DEFS 64
#define MAX_CALLS 64

instruction iStack[MAX_ISTACK];
instruction* I;
int32_t vStack[MAX_VSTACK];
int32_t* V;
uint8_t* rStack[MAX_CALLS];
uint8_t** R;
uint8_t* PC;
const char* err;

uint8_t* defs[MAX_DEFS];
uint8_t nargs[MAX_DEFS];

uint8_t code[] = {
  Do,
    Add,
      Add,
        Add,
          If, 0, 10,
          If, 1, 10,
        Add,
          If, 0, 10, Else, 20,
          If, 1, 10, Else, 20,
      Add,
        Add,
          If, 0, 10, ElIf, 0, 5,
          If, 1, 10, ElIf, 1, 17,
        Add,
          If, 0, 10, ElIf, 0, 2, Else, 20,
          If, 1, 10, ElIf, 0, 2, Else, 20,
    Or, Sub, 10, 10, Mul, 10, 20,
    Mul, Add, 1, 2, Add, 3, 4,
    34,
    Do, 1, 2, 3, 4, 5, End,
    Mul, 10, 20,
    And, 10, 0,
    And, 0, 20,
    And, 30, 40,
    Or, 10, 0,
    Or, 0, 20,
    Or, 30, 40,
  End, 0
};

bool skip() {
  if (*PC < 128) {
    ++PC;
    // TODO: parse larger numbers
    return true;
  }
  if (*PC > LAST_OP) {
    uint8_t id = 255 - *PC++;
    if (id >= MAX_DEFS || !defs[id]) {
      err = "Invalid user function in skip";
      return false;
    }
    uint8_t n = nargs[id];
    while (n--) {
      if (!(skip())) return false;
    }
    return true;
  }
  switch((instruction)*PC++) {
    case Add: case Sub: case Mul: case Div: case Mod: case And: case Or:
      return skip() && skip();
    case Def:
      PC += 2; return skip();
    case Neg:
      return skip();
    case Do: {
      int depth = 1;
      while (depth) {
        if (*PC == End) {
          PC++;
          depth--;
          continue;
        }
        if (!skip()) return false;
      }
      return true;
    }
    case If: {
      if (!(skip() && skip())) return false;
      while (*PC == ElIf) {
        ++PC;
        if (!(skip() && skip())) return false;
      }
      if (*PC == Else) {
        ++PC;
        if (!skip()) return false;
      }
      return true;
    }
    case Empty: case Doing: case End: case Then: case ElIf: case Else: case Call:
      err = "Invalid instruction in skip";
      return false;
  }
}

bool fetch() {
  if (*PC < 128) {
    printf("%td: %"PRIu8"\n", PC - code, *PC);
    *++V = *PC++;
    // TODO: handle numbers larger than 127
    return true;
  }
  printf("%td: %s\n", PC - code, names[*PC - 128]);
  switch(*PC) {
    case Add: case Sub: case Mul: case Div: case Mod:
      *++I = *PC++;
      *++I = Empty;
      *++I = Empty;
      return true;
    case Neg: case And: case Or: case If:
      *++I = *PC++;
      *++I = Empty;
      return true;
    case Do:
      *++I = *PC++;
      return true;
    case Def: {
      PC++;
      uint8_t id = *PC++;
      uint8_t n = *PC++;
      if (id < 0 || id >= MAX_DEFS) {
        err = "User function index out of range";
        return false;
      }
      if (defs[id]) {
        if (nargs[id] != n) {
          err = "Cannot change arity when redefining function";
          return false;
        }
        free(defs[id]);
      }
      else {
        nargs[id] = n;
      }
      uint8_t* start = PC;
      if (!skip()) return false;
      size_t len = PC - start + 1;
      defs[id] = malloc(len);
      memcpy(defs[id], start, len);
      return true;
    }

    default:
      err = "Invalid Instruction in eval";
      return false;
  }
}

bool step() {
  printf("iStack:");
  instruction* i;
  for (i = iStack; i <= I; i++) {
    printf(" %s", names[*i - 128]);
  }
  printf("\nvStack:");
  int32_t* v;
  for (v = vStack; v <= V; v++) {
    printf(" %"PRId32, *v);
  }
  printf("\n");
  if (I < iStack) return false;
  // Switch on instruction at top of iStack
  if (*I > LAST_OP) {
    uint8_t id = 255 - *I--;
    if (id > MAX_DEFS || !defs[id]) {
      err = "No such user function";
      return false;
    }
    uint8_t n = nargs[id];

  }
  switch(*I--) {
    case Empty: if (!fetch()) return false; break;
    // Simple math operators
    case Add: V--; *V += *(V + 1); break;
    case Sub: V--; *V -= *(V + 1); break;
    case Mul: V--; *V *= *(V + 1); break;
    case Div: V--; *V /= *(V + 1); break;
    case Mod: V--; *V %= *(V + 1); break;
    case Neg: *V = -*V; break;
    case Do:
      // If End if reached, let Do drop and move on.
      if (*PC == End) { PC++; break; }
      // Transform to Doing and start new fetch.
      *++I = Doing; if (!fetch()) return false; break;
    case Doing:
      // If End if reached, let Doing drop and move on.
      if (*PC == End) { PC++; break; }
      // Otherwise preserve Doing and drop last value and start new fetch.
      V--; I++; if (!fetch()) return false; break;
    case And:
      // If first value is truthy, drop it and capture second value.
      if (*V) { --V; if (!fetch()) return false; break; }
      // Otherwise keep false and skip second value.
      if (!skip()) return false; break;
    case Or:
      // If the first value was truthy, keep it and skip second value.
      if (*V) { if (!skip()) return false; break; }
      // Otherwise, throw away first value and capture second.
      --V; if (!fetch()) return false; break;
    case If:
      // When condition is true, setup Then/Empty to capture value
      if (*V--) { *++I = Then; if (!fetch()) return false; break; }
      // Skip the body when condition is false.
      if (!skip()) return false;
      // If there is an ElIf, transform it to an If and start over.
      if (*PC == ElIf) { PC++; *++I = If; if (!fetch()) return false; break; }
      // If there is an Else, start next fetch to capture it.
      if (*PC == Else) { PC++; if (!fetch()) return false; break; }
      // Otherwise keep falsy condition as result value.
      V++; break;
    case Then:
      // Skip any ElIf blocks, if any.
      while (*PC == ElIf) { PC++; if (!(skip() && skip())) return false; }
      // Skip Else block if there is one.
      if (*PC == Else) { PC++; if (!skip()) return false; }
      break;
    case Call: {
      uint8_t id = 255 - *I--;
      uint8_t n = nargs[id];
      *R++ = PC;
      PC = defs[id];
      // TODO: call
      break;
    }
    case End: case ElIf: case Else: case Def:
      err = "Invalid instruction in iStack";
      return false;
  }
  return true;
}

int main() {
  I = iStack;
  V = vStack - 1;
  R = rStack - 1;
  *I = Empty;
  PC = code;
  while (step());

  if (err) {
    printf("Error: %s\n", err);
    return -1;
  }
  return 0;
}
