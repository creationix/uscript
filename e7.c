#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

typedef enum {
  Empty = 128,
  Add, Sub, Mul, Div, Mod, Neg,
  Do, Doing, End,
  And, Or,
  If, Then, ElIf, Else,
} instruction;

const char* names[] = {
  "Empty",
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Do", "Doing", "End",
  "And", "Or",
  "If", "Then", "ElIf", "Else",
};

instruction iStack[400];
instruction* I;
int32_t vStack[100];
int32_t* V;
uint8_t* PC;
const char* err;

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
  End
};

bool skip() {
  if (*PC < 128) {
    ++PC;
    // TODO: parse larger numbers
    return true;
  }
  switch(*PC++) {
    case Add: case Sub: case Mul: case Div: case Mod: case And: case Or:
      return skip() && skip();
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
    default:
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
  switch(*I--) {
    case Doing:
      if (*PC == End) {
        PC++;
        break;
      }
      V--;
    case Do:
      *++I = Doing;
    case Empty:
      if (!fetch()) return false;
      break;
    case Add:
      V--;
      *V += *(V + 1);
      break;
    case Sub:
      V--;
      *V -= *(V + 1);
      break;
    case Mul:
      V--;
      *V *= *(V + 1);
      break;
    case Div:
      V--;
      *V /= *(V + 1);
      break;
    case Mod:
      V--;
      *V %= *(V + 1);
      break;
    case Neg:
      *V = -*V;
      break;
    case And:
      if (*V) {
        --V;
        *++I = Empty;
        break;
      }
      if (!skip()) return false;
      break;
    case Or:
      if (!*V) {
        --V;
        *++I = Empty;
        break;
      }
      if (!skip()) return false;
      break;
    case If:
      if (!*V) {
        if (!skip()) return false;
        if (*PC == ElIf) {
          PC++;
          V--;
          *++I = If;
          *++I = Empty;
          break;
        }
        if (*PC == Else) {
          PC++;
          V--;
          *++I = Empty;
          break;
        }
        break;
      }
      V--;
      *++I = Then;
      *++I = Empty;
      break;
    case Then:
      while (*PC == ElIf) {
        PC++;
        if (!(skip() && skip())) return false;
      }
      if (*PC == Else) {
        PC++;
        if (!skip()) return false;
      }
      break;
    case End: case ElIf: case Else:
      err = "Invalid instruction in iStack";
      return false;
  }
  return true;
}

int main() {
  I = iStack;
  V = vStack - 1;
  *I = Empty;
  PC = code;
  while (step());

  if (err) {
    printf("Error: %s\n", err);
    return -1;
  }
  return 0;
}
