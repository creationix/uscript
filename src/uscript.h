#ifndef USCRIPT_H
#define USCRIPT_H
// Feel free to override these to fit your constraints / needs.
#include "stdint.h"

extern void printString(const char* str);
extern void printInt(int val);
extern void slotWrite(int addr, int slot, int value);
extern int slotRead(int i);
// extern void pinMode(int pin, int mode);
// extern void digitalWrite(int pin, int value);
// extern int digitalRead(int pin);
// extern void analogWrite(int pin, int value);
// extern int analogRead(int pin);
// extern void delay(int ms);
// extern void delayMicroseconds(int us);

typedef enum {
  // stack operations
  DROP = 128, // n ->
  DUP, // n -> n n
  DUP2, // a b -> a b a b
  OVER, // a b -> a b a
  SWAP, // a b -> b a
  ROT,  // a b c -> b c a
  ADD, SUB, MUL, DIV, MOD, NEG, // ([a], b)
  BAND, BOR, BXOR, BNOT, RSHIFT, LSHIFT, // (a, b)
  BGET, // (num, bit)
  BSET, // (num, bit)
  BCLR, // (num, bit)
  AND, OR, NOT, XOR, // ([a], b)
  GT, LT, GTE, LTE, EQ, NEQ, // (a, b)
  PM, // (pin, mode)
  DW, // (pin, value)
  DR, // (pin)
  AW, // (pin, value)
  AR, // (pin)
  SW, // (addr/slot, value)
  SR, // (slot)
  DELAY, // (ms)
  UDELAY, // (us)
  SRAND, // (seed)
  RAND, // (mod)
  ISTC, ISFC, IST, ISF, JMP, // ([cond], jump)
  CALL, // (out/in, jump)
  END, // return
  DUMP,
} opcode;

void dump();
void reset();
int eval(unsigned char* pc);

#endif
