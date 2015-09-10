#include "uscript.c"
#include <stdio.h>
#include <assert.h>

state_t S;

int main() {
  printf("%td\n", sizeof(state_t));
  assert(sizeof(state_t) < 1500);
  coroutine_create(&S, (uint8_t[]){
    MUL, ADD,  10, 2, DO, YIELD, ADD, 3, 4, END
  });
  coroutine_create(&S, (uint8_t[]){
    ADD, ADD,  30, MUL, DO, YIELD, 2, END, 3, 4
  });
  // coroutine_create(&S, (uint8_t[]){
  //   XOR, 0, 0
  // });
  // coroutine_create(&S, (uint8_t[]){
  //   DW, 13, NOT, DR, 13
  // });
  // coroutine_create(&S, (uint8_t[]){
  //   ADD,
  //     OR,
  //       1,
  //       AND,
  //         IF, 0, 10,
  //          ELIF, 0, 20,
  //          ELSE, 30,
  //         IF, 1, 40,
  //          ELIF, 0, 50,
  //          ELSE, 60,
  //     41
  // });
  // coroutine_create(&S, (uint8_t[]){
  //   ADD,
  //     OR,
  //       0,
  //       DO, 1, DO, 2, END, 3, END,
  //     2
  // });
  while (loop(&S));
  return 0;
}
