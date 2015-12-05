#include <stdint.h>

#define USCRIPT_VERSION 1
typedef enum {
  Mode = 128, // pinMode(pin, mode)
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
  Do, End, // do ... end
  Add, Sub, Mul, Div, Mod, Neg,
  Band, Bor, Bxor, Bnot, Lshift, Rshift,
  And, Or, Xor, Not, Choose,
  Gt, Gte, Lt, Lte, Eq, Neq,
  Srand, // deadbeef_srand(seed)
  Rand, // deadbeef_rand(modulus)
  Restart, ChipId, FlashChipId, CycleCount,
} opcode_t;

uint8_t* eval(int32_t* stack, uint8_t* pc, int32_t* value);
