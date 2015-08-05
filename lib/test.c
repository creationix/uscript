#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define NUMBER_TYPE int64_t
#include "vm.c"


void test_expression(unsigned char* code, integer expected) {
  unsigned char* end;
  integer actual;
  pc = code;
  skip();
  end = pc;
  pc = code;
  actual = eval();
  assert(pc == end);
  printf("<");
  unsigned char* i;
  for (i = code; i < pc; i++) {
    printf(" %02x", *i);
  }
  printf(" > -> %"PRId64"/%"PRId64"\n", actual, expected);
  assert(actual == expected);
}

void test_program(unsigned char* code, int count, integer* expected) {
  pc = code;
  exec();
  printf("<");
  unsigned char* i;
  for (i = code; i < pc; i++) {
    printf(" %02x", *i);
  }
  printf(" >\n");
  int j;
  for (j = 0; j < count; j++) {
    printf("  %d: %"PRId64"/%"PRId64"\n", j, slots[j], expected[j]);
  }
  for (j = 0; j < count; j++) {
    assert(slots[j] == expected[j]);
  }
}

int main() {
  test_expression((unsigned char[]){ 0 }, 0);
  test_expression((unsigned char[]){ 10 }, 10);
  test_expression((unsigned char[]){ 0x40, 0x64 }, 100);
  test_expression((unsigned char[]){ 0x40, 0xce, 0x10 }, 10000);
  test_expression((unsigned char[]){ 0x43, 0xdc, 0xeb, 0x94, 0x00 }, 1000000000);
  int i = 0;
  for (i = 0; i < 16; i++) {
    integer val = 42 + i;
    slots[i] = 42 + i;
    test_expression((unsigned char[]){ R0 + i }, val);
  }
  test_expression((unsigned char[]){ ADD, 10, 20 }, 30);
  test_expression((unsigned char[]){ SUB, 10, 20 }, -10);
  test_expression((unsigned char[]){ MUL, 10, 20 }, 200);
  test_expression((unsigned char[]){ DIV, 10, 3 }, 3);
  test_expression((unsigned char[]){ MOD, 10, 3 }, 1);
  test_expression((unsigned char[]){ NEG, 0x40, 0x64 }, -100);
  test_expression((unsigned char[]){ EQ, 1, 2 }, 0);
  test_expression((unsigned char[]){ EQ, 3, 3 }, 1);
  test_expression((unsigned char[]){ NEQ, 1, 2 }, 1);
  test_expression((unsigned char[]){ NEQ, 3, 3 }, 0);
  test_expression((unsigned char[]){ GT, 2, 3 }, 0);
  test_expression((unsigned char[]){ GT, 3, 3 }, 0);
  test_expression((unsigned char[]){ GT, 4, 3 }, 1);
  test_expression((unsigned char[]){ LT, 2, 3 }, 1);
  test_expression((unsigned char[]){ LT, 3, 3 }, 0);
  test_expression((unsigned char[]){ LT, 4, 3 }, 0);
  test_expression((unsigned char[]){ GTE, 2, 3 }, 0);
  test_expression((unsigned char[]){ GTE, 3, 3 }, 1);
  test_expression((unsigned char[]){ GTE, 4, 3 }, 1);
  test_expression((unsigned char[]){ LTE, 2, 3 }, 1);
  test_expression((unsigned char[]){ LTE, 3, 3 }, 1);
  test_expression((unsigned char[]){ LTE, 4, 3 }, 0);
  test_expression((unsigned char[]){ AND, 0, 10 }, 0);
  test_expression((unsigned char[]){ AND, 20, 10 }, 10);
  test_expression((unsigned char[]){ OR, 0, 10 }, 10);
  test_expression((unsigned char[]){ OR, 20, 10 }, 20);
  test_expression((unsigned char[]){ XOR, 0, 10 }, 10);
  test_expression((unsigned char[]){ XOR, 20, 10 }, 0);
  test_expression((unsigned char[]){ XOR, 20, 0 }, 20);
  test_expression((unsigned char[]){ XOR, 0, 0 }, 0);
  test_expression((unsigned char[]){ NOT, 0 }, 1);
  test_expression((unsigned char[]){ NOT, 15 }, 0);
  test_expression((unsigned char[]){ IF, 1, 2, 0 }, 2);
  test_expression((unsigned char[]){ IF, 0, 2, 0 }, 0);
  test_expression((unsigned char[]){ IF, 1, 2, ELSE, 3 }, 2);
  test_expression((unsigned char[]){ IF, 0, 2, ELSE, 3 }, 3);
  test_expression((unsigned char[]){ IF, 1, 2, ELIF, 1, 3, 0 }, 2);
  test_expression((unsigned char[]){ IF, 1, 2, ELIF, 0, 3, 0 }, 2);
  test_expression((unsigned char[]){ IF, 0, 2, ELIF, 1, 3, 0 }, 3);
  test_expression((unsigned char[]){ IF, 0, 2, ELIF, 0, 3, 0 }, 0);
  test_expression((unsigned char[]){ IF, 1, 2, ELIF, 1, 3, ELSE, 4 }, 2);
  test_expression((unsigned char[]){ IF, 1, 2, ELIF, 0, 3, ELSE, 4 }, 2);
  test_expression((unsigned char[]){ IF, 0, 2, ELIF, 1, 3, ELSE, 4 }, 3);
  test_expression((unsigned char[]){ IF, 0, 2, ELIF, 0, 3, ELSE, 4 }, 4);
  test_expression((unsigned char[]){ IF, 0, 2, ELIF, 0, 3, ELIF, 1, 4 }, 4);
  test_expression((unsigned char[]){
    ADD,
      MUL, 1, SUB, 6, 3,
      DIV, 0x40, 0x64, 3
  }, 36);

  test_program((unsigned char[]){
    W0, 0x40, 0x64,    // a = 100
    W1, 0,             // b = 0
    L0,                // :start:
    RADD, 0x10,        // b += a
    DECR, 0x01,        // a -= 1
    IS, 0x00,          // if a goto :start:
    END
  }, 2, (integer[]){
    0, 5050
  });

  test_program((unsigned char[]){
    W0, 0x40, 0xce, 0x10,    // a = 10000
    W1, 0,             // b = 0
    L0,                // :start:
    RADD, 0x10,        // b += a
    DECR, 0x01,        // a -= 1
    IS, 0x00,          // if a goto :start:
    END
  }, 2, (integer[]){
    0, 50005000
  });

  // test_program((unsigned char[]){
  //   W0, 0x43, 0xdc, 0xeb, 0x94, 0x00, // a = 1000000000
  //   W1, 0x0,                          // b = 0
  //   L0,                               // :start:
  //   RADD, 0x10,                       // b += a
  //   DECR, 0x01,                       // a -= 1
  //   IS, 0x00,                         // if a goto :start:
  //   END,
  // }, 2, (integer[]){
  //   0, 500000000500000000
  // });

  i = add_string("Hello World");
  i = add_string("happy");
  i = add_string("Stuff");
  i = add_string("happy");
  i = add_string("more");
  const char* s = strings;
  while (*s) {
    printf("%s\n", s);
    while (*s++);
  }


  return 0;
}
//
//
//
// // #!/usr/bin/tcc -run -Wall -Werror
//
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <inttypes.h>
// #include <assert.h>
//
// #include "uscript.c"
//
// #define KNRM  "\x1B[0m"
// #define KRED  "\x1B[31m"
// #define KGRN  "\x1B[1;32m"
// #define KYEL  "\x1B[33m"
// #define KBLU  "\x1B[1;34m"
// #define KMAG  "\x1B[1;35m"
// #define KCYN  "\x1B[36m"
// #define KWHT  "\x1B[37m"
//
// static unsigned char* Times42(struct uState* S, unsigned char* pc, number* res) {
//   if (!res) return skip(S, pc);
//   pc = eval(S, pc, res);
//   *res *= 42;
//   return pc;
// }
//
// static struct uState S;
//
// static struct user_func funcs[] = {
//   {"Times-42", Times42},
//   {NULL}
// };
//
// static void test_raw(uint8_t* code, int len, number answer) {
//   printf(KBLU "< ");
//   for (int i = 0; i < len; i++) {
//     printf("%02x ", code[i]);
//   }
//   printf(">" KNRM);
//   number result;
//   int used = eval(&S, code, &result) - code;
//   printf(" %s(%d/%d)\n%s%"PRId64"%s\n", KWHT, used, len, KYEL, result, KNRM);
//   assert(used == len);
//   assert(result == answer);
// }
//
// static void test(char* code, number answer) {
//   printf(KGRN "%s\n" KNRM, code);
//   uint8_t* program = (uint8_t*)malloc(strlen(code) + 1);
//   memcpy(program, code, strlen(code) + 1);
//   int len = compile(&S, program);
//   if ((int) len < 0) {
//     int offset = 1 - (int)len;
//     printf("Unexpected input at %d: '%s'\n", offset, program + offset);
//     assert(0);
//   }
//
//   test_raw(program, len, answer);
//   free(program);
// }
//
// int main() {
//
//   S.malloc = malloc;
//   S.free = free;
//   S.funcs = funcs;
//   S.num_funcs = 0;
//   while (funcs[S.num_funcs].name) S.num_funcs++;
//
//   assert(name_to_op(&S, "BADNAME", 7) == 0);
//
//   test("0x100", 0x100);
//   test("0xdeadbeeef", 0xdeadbeeef);
//   test("0XDEADBEEF", 0XDEADBEEF);
//   test("Times-42 10", 420);
//   test("DO 3 SET i 10 AND 0 Times-42 SET i 20 GET i", 10);
//   test("DO 3 SET i 10 AND 1 Times-42 SET i 20 GET i", 20);
//   test("DO 2 SRAND 42 RAND 1000", 695);
//   test("SET ADD 1 2 4", 4);
//   test("GET SUB 4 1", 4);
//   test("POKE 0 10", 10);
//   test("PEEK 0", 10);
//   test("POKE 1 20", 20);
//   test("PEEK 1", 20);
//   test("INSERT 0 10", 10);
//   test("INSERT 0 20", 20);
//   test("ADD READ 0 READ 1", 30);
//   test("WRITE 0 30", 30);
//   test("ADD READ 0 READ 1", 40);
//   test("WRITE 1 40", 40);
//   test("ADD READ 0 READ 1", 70);
//   test("REMOVE 0", 30);
//   test("READ 0", 40);
//   test("NOT 42", 0);
//   test("OR 7 SET b NEG 2", 7);
//   test("GET b", 0);
//   test("AND 7 SET b NEG 2", -2);
//   test("XOR 5 0", 5);
//   test("XOR 0 7", 7);
//   test("XOR 10 14", 0);
//   test("XOR 0 0", 0);
//   test("BNOT 42", -43);
//   test("BAND 43 34", 34);
//   test("BOR 43 34", 43);
//   test("BXOR 43 34", 9);
//   test("SET a 5", 5);
//   test("GET a", 5);
//   test("LT 1 2", 1);
//   test("GTE 1 2", 0);
//   test("EQ 1 2", 0);
//   test("NEQ 1 2", 1);
//   test("ADD 1 MUL 2 3", 7);
//   test("MUL SUB 1 2 3", -3);
//   test("DIV 10 3", 3);
//   test("MOD 10 3", 1);
//   test("1", 1);
//   test("10", 10);
//   test("100", 100);
//   test("1000", 1000);
//   test("10000", 10000);
//   test("100000", 100000);
//   test("1000000", 1000000);
//   test("10000000", 10000000);
//   test("100000000", 100000000);
//   test("1000000000", 1000000000);
//   test("10000000000", 10000000000);
//   test("100000000000", 100000000000);
//   test("1000000000000", 1000000000000);
//   test("10000000000000", 10000000000000);
//   test("100000000000000", 100000000000000);
//   test("1000000000000000", 1000000000000000);
//   test("10000000000000000", 10000000000000000);
//   test("100000000000000000", 100000000000000000);
//   test("1000000000000000000", 1000000000000000000);
//   test("NEG 1000000000", -1000000000);
//   test("IF 1 9", 9);
//   test("IF 0 9", 0);
//   test("IF 11 9 ELSE 5", 9);
//   test("IF 0 9 ELSE 5", 5);
//   test("IF 14 9 ELIF 0 3 ELSE 5", 9);
//   test("IF 0 9 ELIF 0 3 ELSE 5", 5);
//   test("IF 0 9 ELIF 1 3 ELSE 5", 3);
//   test("IF 0 9 ELIF 1 3", 3);
//   test("MATCH 42 WHEN 42 7", 7);
//   test("MATCH 42 WHEN 34 7", 0);
//   test("MATCH 42 WHEN 34 7 WHEN 42 9", 9);
//   test("MATCH 42 WHEN 34 7 ELSE 5", 5);
//   test("SET i 10", 10);
//   test("WHILE GET i DECR i 1", 0);
//   test("SET s 0", 0);
//   test(
//     "DO 2\n"
//     "  SET i 0\n"
//     "  SET s 0\n", 0);
//   test(
//     "WHILE LT GET i 10 DO 2\n"
//     "  INCR i 1\n"
//     "  SET s ADD GET s GET i", 55);
//   test("WHILE DECR i 1 SET s ADD GET s GET i", 100);
//   test("FOR i 1 10 INCR s GET i", 155);
//   test("DEF a ADD GET a 10", 0);
//   test("SET a 10", 10);
//   test("RUN a", 20);
//   test("RM a", 0);
//   test("RUN a", 0);
//   test(
//     "DEF s DO 2\n"
//     "  SET s 0\n"
//     "  FOR i 1 GET m\n"
//     "    INCR s GET i", 18);
//
//   test("SET i 1000000000000000000", 1000000000000000000);
//   test("WAIT NOT DECR i 100000000000000", 1);
//   test("DO 2 SET m 100 RUN s", 5050);
//   test("DO 2 SET m 1000 RUN s", 500500);
//   test("DO 2 SET m 10000 RUN s", 50005000);
//   test("DO 2 SET m 100000 RUN s", 5000050000);
//   test("DO 2 SET m 1000000 RUN s", 500000500000);
//   test("DO 2 SET m 10000000 RUN s", 50000005000000);
//   test("DO 2 SET m 100000000 RUN s", 5000000050000000);
//   // test("DO 2 SET m 1000000000 RUN s", 500000000500000000);
//
//   return 0;
// }
