#define VERBOSE_VM
#define MAX_COROUTINES 14

#include "uscript.c"
#include <stdio.h>
#include <assert.h>

state_t S;

typedef struct {
  uint8_t* code;
  int32_t expected;
} unit_test_t;

void test(unit_test_t* tests, int len) {
  printf("\nTesting %d coroutine(s)\n", len);
  int i;
  for (i = 0; i < len; i++) {
    coroutine_create(&S, tests[i].code);
  }
  while (loop(&S, 1000)) printf("\nTICK\n");
  for (i = 0; i < len; i++) {
    coroutine_t* T = S.coroutines + i;
    assert(T->v == T->vstack);
    assert(*T->v == tests[i].expected);
  }
}

int main() {

  printf("%td\n", sizeof(state_t));
  assert(sizeof(state_t) <= 2608);

  // Test basic aritmetic with yields to interleave work.
  test((unit_test_t[]){
    {(uint8_t[]){ MUL, ADD,  1, 2, DO, YIELD, ADD, 3, 4, END }, 21},
    {(uint8_t[]){ DIV, DO, YIELD, SUB,  1, 2, END, SUB, 3, 4 }, 1},
    {(uint8_t[]){ MOD, ADD, NEG, 3, DO, YIELD, 10, END, 5 }, 2},
  }, 3);

  // Test return early exit
  test((unit_test_t[]){{(uint8_t[]){
    ADD,
      DO, 1, RETURN, 2, 3, END,
      DO, 1, RETURN, 2, END
  }, 4}}, 1);

  // Test delay and logic
  test((unit_test_t[]){
    {(uint8_t[]){ XOR, DELAY, 30, 0 }, 30},
    {(uint8_t[]){ XOR, 0, DELAY, 40 }, 40},
    {(uint8_t[]){ XOR, 0, 0 }, 0},
    {(uint8_t[]){ XOR, DELAY, 15, DELAY, 25 }, 0},
    {(uint8_t[]){ AND, DELAY, 10, DELAY, 20 }, 20},
    {(uint8_t[]){ AND, 0, DELAY, 20 }, 0},
    {(uint8_t[]){ AND, DELAY, 10, 0 }, 0},
    {(uint8_t[]){ AND, 0, 0 }, 0},
    {(uint8_t[]){ OR, DELAY, 10, DELAY, 20 }, 10},
    {(uint8_t[]){ OR, 0, DELAY, 20 }, 20},
    {(uint8_t[]){ OR, DELAY, 10, 0 }, 10},
    {(uint8_t[]){ OR, 0, 0 }, 0},
    {(uint8_t[]){ NOT, DELAY, 10 }, 0},
    {(uint8_t[]){ NOT, 0 }, 1},
  }, 14);

  // Test fake GPIO functions
  test((unit_test_t[]){
    {(uint8_t[]){
      DO,
        PM, 13, 1,
        PM, 12, 1,
        YIELD,
        DW, 13, NOT, DR, 13,
        AW, 12, NOT, AR, 12,
      END,
    }, 1},
  }, 1);

  // Test less than
  test((unit_test_t[]){
    {(uint8_t[]){ LT, 1, 2 }, 1},
    {(uint8_t[]){ LT, 2, 1 }, 0},
    {(uint8_t[]){ LT, 3, 3 }, 0},
    {(uint8_t[]){ LTE, 1, 2 }, 1},
    {(uint8_t[]){ LTE, 2, 1 }, 0},
    {(uint8_t[]){ LTE, 3, 3 }, 1},
  }, 6);

  // Test greater than
  test((unit_test_t[]){
    {(uint8_t[]){ GT, 1, 2 }, 0},
    {(uint8_t[]){ GT, 2, 1 }, 1},
    {(uint8_t[]){ GT, 3, 3 }, 0},
    {(uint8_t[]){ GTE, 1, 2 }, 0},
    {(uint8_t[]){ GTE, 2, 1 }, 1},
    {(uint8_t[]){ GTE, 3, 3 }, 1},
  }, 6);

  // Test equality
  test((unit_test_t[]){
    {(uint8_t[]){ EQ, 1, 2 }, 0},
    {(uint8_t[]){ EQ, 3, 3 }, 1},
    {(uint8_t[]){ NEQ, 1, 2 }, 1},
    {(uint8_t[]){ NEQ, 3, 3 }, 0},
  }, 4);

  // Test if/elif/else
  test((unit_test_t[]){
    {(uint8_t[]){ IF, DO, YIELD, 1, END, 10 }, 10},
    {(uint8_t[]){ IF, 0, DO, YIELD, 1, 10, END }, 0},
    {(uint8_t[]){ IF, 1, 10, ELSE, DO, YIELD, 20, END }, 10},
    {(uint8_t[]){ IF, DO, YIELD, 0, END, 10, ELSE, 20 }, 20},
    {(uint8_t[]){ IF, 1, 10, ELIF, DO, YIELD, 1, END, 20 }, 10},
    {(uint8_t[]){ IF, 0, 10, ELIF, DO, YIELD, 1, END, 20 }, 20},
    {(uint8_t[]){ IF, 0, 10, ELIF, 0, 20, ELSE, DO, YIELD, 30, END }, 30},
  }, 7);

  // Test skip if
  test((unit_test_t[]){
    {(uint8_t[]){
      ADD, OR, 1,
        IF, 0, YIELD, ELIF, 1, 2, ELSE, 3,
      9
    }, 10}
  }, 1);

  // Test srand/rand
  test((unit_test_t[]){
    {(uint8_t[]){ SRAND, 10 }, 10},
    {(uint8_t[]){ RAND, 50, END }, 41},
    {(uint8_t[]){ RAND, 50, END }, 29},
    {(uint8_t[]){ RAND, 50, END }, 38},
    {(uint8_t[]){ RAND, 50, END }, 2},
    {(uint8_t[]){ RAND, 50, END },  8},
  }, 6);

  // Test wait
  test((unit_test_t[]){
    {(uint8_t[]){ SRAND, 11}, 11},
    {(uint8_t[]){ WAIT, GT, RAND, 50, 40 }, 1},
    {(uint8_t[]){ WAIT, GT, DELAY, RAND, 50, 30 }, 1},
  }, 3);

  return 0;
}
