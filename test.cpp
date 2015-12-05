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

  eval(stack, (uint8_t[]){Call, 0, Func, Do,
    // Store the string "Hi" as a pointer in slot 0
    Gset, 0, String, 2,'H','i',
    // Print the length of the string
    Print, Alen, Gget, 0,
    // Print the string
    Aprint, Gget, 0,
    // Free the string and clear the slot.
    Gset, 0, Free, Gget, 0,
  End}, &value);

  printf("\n\n");

  alive = 3;
  eval(stack, (uint8_t[]){Call, 0, Func, Do,
    Gset, 0, Func, Do,
      Mode, D7, 1,
      Write, D7, 0,
      Forever, Do,
        Delay, num(333),
        Write, D7,
          Not, Read, D7,
      End,
    End,
    Call, 0, Gget, 0,
  End}, &value);

  printf("\n\n");

  eval(stack, (uint8_t[]){Call, 0, Func, Do,
    Call, (16 / sizeof(intptr_t)), Gget, 0,
    Gset, 0, Free, Gget, 0,
  End}, &value);

  return 0;
}

buffer_t* copyBuffer(int len, uint8_t* data) {
  buffer_t* buf = (buffer_t*)malloc(sizeof(buffer_t) + len);
  memcpy(buf->data, data, len);
  return buf;
}
