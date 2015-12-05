#include <assert.h>
#include "vm.cpp"

enum nodemcu_pins {
  D0 = 16,
  D1 = 5,
  D2 = 4,
  D3 = 0,
  D4 = 2,
  D5 = 14,
  D6 = 12,
  D7 = 13,
  D8 = 15,
  D9 = 3,
  D10 = 1,
  D11 = 9,
  D12 = 10,
};

int alive = 0;
int isAlive() {
  return --alive > 0;
}

intptr_t stack[100];
intptr_t value;

// Use for numbers between 64 and 8,191
#define num(n) (0x40|(n)>>7),((n)&0x7f)

int main() {
  eval(stack, (uint8_t[]){
    Gset, 0, Func, Do,
      Mode, D7, 1,
      Forever, Do,
        Write, D7,
          Not, Read, D7,
        Delay, num(200),
      End,
    End, 0,
  }, &value);
  printf("Created and stored a fn %p\n\n", (uint8_t*)value);

  alive = 3;
  eval(stack, (uint8_t[]){Do,
    Call, Gget, 0, 16,
    Gset, 0, Free, Gget, 0,
  End}, &value);

  // eval(stack, (uint8_t[]){Do,
  //   Mode, 13, 1,
  //   Write, 13, 1,
  //   Delay, num(1000),
  //   Write, 13, 0,
  //   Delay, num(1000),
  // End}, &value);
  return 0;
}
