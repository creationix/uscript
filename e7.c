#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

typedef enum {
  Empty = 128,
  Add, Sub, Mul, Div, Mod, Neg,
  Do, End,
} instruction;

instruction iStack[400];
instruction* I;
int32_t vStack[100];
int32_t* V;
uint8_t* PC;
const char* err;

uint8_t code[] = {
  Mul, Add, 1, 2, Add, 3, 4
};

bool step() {
  printf("iStack=%td, vStack=%td, pc=%td\n", I - iStack + 1, V - vStack + 1, PC - code);
  if (I < iStack) return false;
  // Switch on instruction at top of iStack
  switch(*I--) {
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
          // TODO: Implement do
          break;
        case End:
          // TODO: Implement end
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
    case End: case Do:
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
  printf("Result: %"PRId32"\n", *V);
  return 0;
}
