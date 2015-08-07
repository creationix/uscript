#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

typedef enum {
  Empty = 128,
  Add, Sub, Mul, Div, Mod, Neg,
  Do, Doing, End,
} instruction;

const char* names[] = {
  "Empty",
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Do", "Doing", "End"
};

instruction iStack[400];
instruction* I;
int32_t vStack[100];
int32_t* V;
uint8_t* PC;
const char* err;

uint8_t code[] = {
  Do,
    Mul, Add, 1, 2, Add, 3, 4,
    34,
    Do, 1, 2, 3, 4, 5, End,
    Mul, 10, 20,
  End
};

bool step() {
  printf("\niStack:");
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
  if (*PC < 128) {
    printf("%td: %"PRIu8"\n", PC - code, *PC);
  }
  else {
    printf("%td: %s\n", PC - code, names[*PC - 128]);
  }
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
        *++V = *PC++;
        // TODO: handle numbers larger than 127
        break;
      }
      switch(*PC) {
        case Add: case Sub: case Mul: case Div: case Mod:
          *++I = *PC++;
          *++I = Empty;
          *++I = Empty;
          break;
        case Neg:
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
