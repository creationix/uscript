// #!/usr/bin/tcc -run -Wall -Werror

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
  int used = eval(code, &result) - code;
  printf(" %s(%d/%d)\n%s%ld%s\n", KWHT, used, len, KYEL, result, KNRM);
  assert(used == len);
  assert(result == answer);
}

static void test(char* code, var answer) {
  printf(KGRN "%s\n" KNRM, code);
  uint8_t* program = (uint8_t*)malloc(strlen(code) + 1);
  memcpy(program, code, strlen(code) + 1);
  int len = compile(program);
  if ((int) len < 0) {
    int offset = 1 - (int)len;
    printf("Unexpected input at %d: '%s'\n", offset, program + offset);
    assert(0);
  }

  test_raw(program, len, answer);
  free(program);
}

int main() {
  test("NOT 42", 0);
  test("OR 7 SET b NEG 2", 7);
  test("GET b", 0);
  test("AND 7 SET b NEG 2", -2);
  test("XOR 5 0", 5);
  test("XOR 0 7", 7);
  test("XOR 10 14", 0);
  test("XOR 0 0", 0);
  test("BNOT 42", -43);
  test("BAND 43 34", 34);
  test("BOR 43 34", 43);
  test("BXOR 43 34", 9);
  test("SET a 5", 5);
  test("GET a", 5);
  test("LT 1 2", 1);
  test("GTE 1 2", 0);
  test("EQ 1 2", 0);
  test("NEQ 1 2", 1);
  test("ADD 1 MUL 2 3", 7);
  test("MUL SUB 1 2 3", -3);
  test("DIV 10 3", 3);
  test("MOD 10 3", 1);
  test("1", 1);
  test("10", 10);
  test("100", 100);
  test("1000", 1000);
  test("10000", 10000);
  test("100000", 100000);
  test("1000000", 1000000);
  test("10000000", 10000000);
  test("100000000", 100000000);
  test("1000000000", 1000000000);
  test("10000000000", 10000000000);
  test("100000000000", 100000000000);
  test("1000000000000", 1000000000000);
  test("10000000000000", 10000000000000);
  test("100000000000000", 100000000000000);
  test("1000000000000000", 1000000000000000);
  test("10000000000000000", 10000000000000000);
  test("100000000000000000", 100000000000000000);
  test("1000000000000000000", 1000000000000000000);
  test("NEG 1000000000", -1000000000);
  test("IF 1 9", 9);
  test("IF 0 9", 0);
  test("IF 11 9 ELSE 5", 9);
  test("IF 0 9 ELSE 5", 5);
  test("IF 14 9 ELIF 0 3 ELSE 5", 9);
  test("IF 0 9 ELIF 0 3 ELSE 5", 5);
  test("IF 0 9 ELIF 1 3 ELSE 5", 3);
  test("IF 0 9 ELIF 1 3", 3);
  test("MATCH 42 WHEN 42 7", 7);
  test("MATCH 42 WHEN 34 7", 0);
  test("MATCH 42 WHEN 34 7 WHEN 42 9", 9);
  test("MATCH 42 WHEN 34 7 ELSE 5", 5);
  test("SET i 10", 10);
  test("WHILE GET i DECR i 1", 0);
  test("SET s 0", 0);
  test(
    "DO 2\n"
    "  SET i 0\n"
    "  SET s 0\n", 0);
  test(
    "WHILE LT GET i 10 DO 2\n"
    "  INCR i 1\n"
    "  SET s ADD GET s GET i", 55);
  test("WHILE DECR i 1 SET s ADD GET s GET i", 100);
  test("FOR i 1 10 INCR s GET i", 155);
  test("DEF a ADD GET a 10", 0);
  test("SET a 10", 10);
  test("RUN a", 20);
  test("RM a", 0);
  test("RUN a", 0);
  test(
    "DEF s DO 2\n"
    "  SET s 0\n"
    "  FOR i 1 GET m\n"
    "    INCR s GET i", 18);

  test("SET i 100000000000000000", 100000000000000000);
  test("WAIT NOT DECR i 10000000000000", 1);
  test("DO 2 SET m 100 RUN s", 5050);
  test("DO 2 SET m 1000 RUN s", 500500);
  test("DO 2 SET m 10000 RUN s", 50005000);
  test("DO 2 SET m 100000 RUN s", 5000050000);
  test("DO 2 SET m 1000000 RUN s", 500000500000);
  test("DO 2 SET m 10000000 RUN s", 50000005000000);
  test("DO 2 SET m 100000000 RUN s", 5000000050000000);
  test("DO 2 SET m 1000000000 RUN s", 500000000500000000);
}
