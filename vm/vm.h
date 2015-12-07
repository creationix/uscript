#include <stdint.h>

#define USCRIPT_VERSION 1
typedef enum {
  Do = 128, End, // do ... end
  Dump, // (num)
  Mode, // pinMode(pin, mode)
  Read, // digitalRead(pin)
  Write, // digitalWrite(pin, value)
  Aread, // analogRead(pin)
  Pwrite, // analogWrite(pin, value)
  Ibegin, // Wiring.begin(sda, scl)
  Ifrom, // Wiring.requestFrom(address, quantity, stop)
  Istart, // Wiring.beginTransmission(address)
  Istop, // Wiring.endTransmission(stop)
  Iwrite, // Wiring.write(byte)
  Iavailable, // Wiring.available()
  Iread, // Wiring.read()
  Tone, // tone(pin, frequency, ms)
  Delay, // delay(ms)
  Call, // (stackOffset, codeOffset) codeOffset is int16
  Gosub, // (codeOffset)
  Goto, // (codeOffset)
  Gget, // (var)
  Gset, // (var, value)
  Get, // (var)
  Set, // (var, value)
  Incr, Decr, // (var)
  IncrMod, DecrMod, // (var, mod)
  Forever, // (action)
  While, // (condition, action)
  Wait, // (condition)
  If, // (condition, action)
  ElseIf, // (condition, action)
  Else, // (action)
  Add, Sub, Mul, Div, Mod, Neg,
  Band, Bor, Bxor, Bnot, Lshift, Rshift,
  And, Or, Xor, Not, Choose,
  Gt, Gte, Lt, Lte, Eq, Neq,
  Srand, // deadbeef_srand(seed)
  Rand, // deadbeef_rand(modulus)
  Restart, ChipId, FlashChipId, CycleCount, GetFree,
} opcode_t;

typedef struct {
  int len;
  uint8_t data[];
} buffer_t;

buffer_t* copyBuffer(int len, uint8_t* data);

uint8_t* eval(intptr_t* stack, uint8_t* pc, intptr_t* value);
