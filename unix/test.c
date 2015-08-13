#include "uscript.c"
#include <stdio.h>

static int run(struct state* S, const char* code) {
  function out[100];
  int res = compile(S, out, 100, code);
  if (res >= 0) {
    printf("%s\r\n", code);
    while (res--) printf(" ");
    printf("^ Unexpected input\r\n");
    return -1;
  }
  return 0;
}

int main() {
  struct state S;
  S.fns = (function[]){
    Call, Add, Sub, Mul, Div, Swap, Over, Dup, Decr, Incr,
    IsTrue, IsFalse, Jump, Random, SeedRandom,
    // Dump, Delay, DelayMicroseconds,
    // PinMode, DigitalWrite, DigitalRead, AnalogWrite, AnalogRead,
  };
  S.names =
    "CALL\0ADD\0SUB\0MUL\0DIV\0SWAP\0OVER\0DUP\0DECR\0INCR\0"
    "IST\0ISF\0JMP\0RAND\0SRAND\0"
    // "DUMP\0DELAY\0UDELAY\0"
    // "PM\0DW\0DR\0AW\0AR\0"
    "\0";

  return run(&S, "\
    a0 0 PM       \
    DUMP          \
    a1 1 PM       \
    DUMP          \
    3 1 DW        \
    7 0 DW        \
    a0 AR DELAY   \
    3 0 DW        \
    7 1 DW        \
    a0 AR DELAY   \
    JMP -31       \
  ");

}
