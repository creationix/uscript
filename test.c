#!/usr/bin/tcc -run -Wall -Werror

//#define OP_LOG
#include "uscript.c"
#include <stdio.h>
#include <assert.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

static void test_raw(uint8_t* code, int len, var answer) {
  printf(KBLU "< ");
  for (int i = 0; i < len; i++) {
    printf("%02x ", code[i]);
  }
  printf(">" KNRM);
  var result;
  var used = eval(code, &result) - code;
  printf(" %s(%d/%d)\n%s%d%s\n", KWHT, used, len, KYEL, result, KNRM);
  assert(used == len);
  assert(result == answer);
}

static void test(uint8_t* code, var answer) {
  printf(KGRN "%s\n" KNRM, code);
  int len = compile(code);
  if ((int) len < 0) {
    int offset = 1 - (int)len;
    printf("Unexpected input at %d: '%s'\n", offset, code + offset);
    assert(0);
  }

  test_raw(code, len, answer);
}

int main() {
  test((uint8_t*)"NOT 42", 0);
  test((uint8_t*)"OR 7 SET b NEG 2", 7);
  test((uint8_t*)"GET b", 0);
  test((uint8_t*)"AND 7 SET b NEG 2", -2);
  test((uint8_t*)"XOR 5 0", 5);
  test((uint8_t*)"XOR 0 7", 7);
  test((uint8_t*)"XOR 10 14", 0);
  test((uint8_t*)"XOR 0 0", 0);
  test((uint8_t*)"BNOT 42", -43);
  test((uint8_t*)"BAND 43 34", 34);
  test((uint8_t*)"BOR 43 34", 43);
  test((uint8_t*)"BXOR 43 34", 9);
  test((uint8_t*)"SET a 5", 5);
  test((uint8_t*)"GET a", 5);
  test((uint8_t*)"LT 1 2", 1);
  test((uint8_t*)"GTE 1 2", 0);
  test((uint8_t*)"EQ 1 2", 0);
  test((uint8_t*)"NEQ 1 2", 1);
  test((uint8_t*)"ADD 1 MUL 2 3", 7);
  test((uint8_t*)"MUL SUB 1 2 3", -3);
  test((uint8_t*)"DIV 10 3", 3);
  test((uint8_t*)"MOD 10 3", 1);
  test((uint8_t*)"1", 1);
  test((uint8_t*)"10", 10);
  test((uint8_t*)"100", 100);
  test((uint8_t*)"1000", 1000);
  test((uint8_t*)"10000", 10000);
  test((uint8_t*)"100000", 100000);
  test((uint8_t*)"1000000", 1000000);
  test((uint8_t*)"10000000", 10000000);
  test((uint8_t*)"100000000", 100000000);
  test((uint8_t*)"1000000000", 1000000000);
  test((uint8_t*)"NEG 1000000000", -1000000000);
  test((uint8_t*)"IF 1 9", 9);
  test((uint8_t*)"IF 0 9", 0);
  test((uint8_t*)"IF 11 9 ELSE 5", 9);
  test((uint8_t*)"IF 0 9 ELSE 5", 5);
  test((uint8_t*)"IF 14 9 ELIF 0 3 ELSE 5", 9);
  test((uint8_t*)"IF 0 9 ELIF 0 3 ELSE 5", 5);
  test((uint8_t*)"IF 0 9 ELIF 1 3 ELSE 5", 3);
  test((uint8_t*)"IF 0 9 ELIF 1 3", 3);
  test((uint8_t*)"MATCH 42 WHEN 42 7", 7);
  test((uint8_t*)"MATCH 42 WHEN 34 7", 0);
  test((uint8_t*)"MATCH 42 WHEN 34 7 WHEN 42 9", 9);
  test((uint8_t*)"MATCH 42 WHEN 34 7 ELSE 5", 5);
  test((uint8_t*)"SET i 10", 10);
  test((uint8_t*)"WHILE GET i DECR i 1", 0);
  test((uint8_t*)"SET s 0", 0);
  test((uint8_t*)
    "DO 2\n"
    "  SET i 0\n"
    "  SET s 0\n", 0);
  test((uint8_t*)
    "WHILE LT GET i 10 DO 2\n"
    "  INCR i 1\n"
    "  SET s ADD GET s GET i", 55);
  test((uint8_t*)"WHILE DECR i 1 SET s ADD GET s GET i", 100);
  test((uint8_t*)"FOR i 1 10 INCR s GET i", 155);
}
