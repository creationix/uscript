#include "uscript.h"
#define REPL_BUFFER 512
#define MAX_OPS 100

char line[REPL_BUFFER];
int offset;
function out[MAX_OPS];

struct state S;

function fns[] = {
  Call, Add, Sub, Mul, Div,
  Mod, Neg, Not,
  Swap, Over, Dup, Decr, Incr,
  IsTrue, IsFalse, Jump, Random, SeedRandom,
  Dump, Delay, DelayMicroseconds,
  PinMode, DigitalWrite, DigitalRead, AnalogWrite, AnalogRead,
};
const char* names =
  "CALL\0ADD\0SUB\0MUL\0DIV\0"
  "MOD\0NEG\0NOT\0"
  "SWAP\0OVER\0DUP\0DECR\0INCR\0"
  "IST\0ISF\0JMP\0RAND\0SRAND\0"
  "DUMP\0DELAY\0UDELAY\0"
  "PM\0DW\0DR\0AW\0AR\0"
  "\0";

//  run(&S, "\
//    a0 0 PM       \
//    a1 1 PM       \
//    3 1 PM       \
//    7 1 PM       \
//  ");
//  run(&S, "\
//    3 1 DW        \
//    7 0 DW        \
//    DUMP          \
//    a0 AR DELAY   \
//    3 0 DW        \
//    7 1 DW        \
//    DUMP          \
//    a0 AR DELAY   \
//  ");

void setup() {
  Serial.begin(9600);
  S.fns = fns;
  S.names = names;
  S.top = S.stack - 1;
}

function dumpCode[] = {Dump, 0};

void handle_input(struct state* S, char c) {
  if (offset < REPL_BUFFER && c >= 0x20 && c < 0x7f) {
    line[offset++] = c;
    Serial.write(c);
  }
  else if (offset > 0 && (c == 127 || c == 8)) {
    line[--offset] = 0;
    Serial.print("\x08 \x08");
  }
  else if (c == '\r' || c == '\n') {
    Serial.print("\r\n");
    if (offset) {
      line[offset++] = 0;

      int res = compile(S, out, 100, line);
      if (res >= 0) {
        while (res--) Serial.print(" ");
        Serial.print("  ^ Unexpected input\r\n");
      }
      else {
        int i = 0;
        S->pc = out;
        (*S->pc)(S);
        S->pc = dumpCode;
        (*S->pc)(S);
      }

    }
    offset = 0;
    Serial.print("> ");
  }
}


void loop() {
  while (Serial.available() > 0) handle_input(&S, Serial.read());
}

