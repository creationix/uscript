#include "uscript.c"
#include <stdio.h>
#include <assert.h>

state_t S;

#ifndef ARDUINO
#include "wiring-polyfill.c"
#endif

int main() {
  printf("%td\n", sizeof(state_t));
  assert(sizeof(state_t) < 1500);
  coroutine_create(&S, (uint8_t[]){
    MUL, ADD, 1, 2, YIELD, ADD, 3, 4
  });
  coroutine_create(&S, (uint8_t[]){
    ADD, ADD, 1, MUL, YIELD, 2, 3, 4
  });
  while (loop(&S));
  return 0;
}
