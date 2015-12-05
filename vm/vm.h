#include <stdint.h>

#define USCRIPT_VERSION 1
typedef enum {
  Do = 128, End, // do ... end
  Print, // (num)
  Aprint, // (ptr)
  String, // length prefixed bytes
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
  Delay, // delay(ms)
  Func, // (body)
  Call, // (shift, ptr)
  Alloc, // (size)
  Aget, // (ptr, offset)
  Aset, // (ptr, offset, value)
  Alen, // (ptr)
  Free, // (ptr)
  Gget, // (index)
  Gset, // (index, value)
  Get, // (index)
  Set, // (index, value)
  Incr, Decr, // (index)
  IncrMod, DecrMod, // (index, mod)
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
