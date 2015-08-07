#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

typedef enum {
  Empty = 128,
  Add, Sub, Mul, Div, Mod, Neg,
  Do, Doing, End,
  And, Or
} instruction;

const char* names[] = {
  "Empty",
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Do", "Doing", "End",
  "And", "Or",
};

instruction iStack[400];
instruction* I;
int32_t vStack[100];
int32_t* V;
uint8_t* PC;
const char* err;

uint8_t code[] = {
  Do,
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

void skip() {
  if (*PC < 128) {
    ++PC;
    // TODO: parse larger numbers
    return;
  }
  switch(*PC++) {
    case Add: case Sub: case Mul: case Div: case Mod: case And: case Or:
      skip(); skip();
      return;
    case Neg:
      skip();
      return;
    case Do: {
      int depth = 1;
      while (depth) {
        if (*PC == End) {
          PC++;
          depth--;
        }
        else {
          skip();
        }
      }
    }
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
      if (*PC < 128) {
        printf("%td: %"PRIu8"\n", PC - code, *PC);
        *++V = *PC++;
        // TODO: handle numbers larger than 127
        break;
      }
      printf("%td: %s\n", PC - code, names[*PC - 128]);
      switch(*PC) {
        case Add: case Sub: case Mul: case Div: case Mod:
          *++I = *PC++;
          *++I = Empty;
          *++I = Empty;
          break;
        case Neg: case And: case Or:
          *++I = *PC++;
          *++I = Empty;
          break;
        case Do:
          *++I = *PC++;
          break;
        default:
          err = "Invalid Instruction";
          return false;
      }
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
      skip();
      break;
    case Or:
      if (!*V) {
        --V;
        *++I = Empty;
        break;
      }
      skip();
      break;
    case End:
      // You should never get here since these aren't operators.
      assert(0);
      break;
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
