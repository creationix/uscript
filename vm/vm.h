#include <stdint.h>

#define USCRIPT_VERSION 1
typedef enum {
  Mode = 128, // (pin, mode)
  Read, // (pin)
  Write, // (pin, value)
  Aread, // (pin)
  Pwrite, // (pin, value)
  Ibegin, // Wiring.begin(sda, scl)
  Ifrom, // Wiring.requestFrom(address, quantity, stop)
  Istart, // Wiring.beginTransmission(address)
  Istop, // Wiring.endTransmission(stop)
  Iwrite, // Wiring.write(byte)
  Iavailable, // Wiring.available()
  Iread, // Wiring.read()
  Delay, // (ms)
  Forever, // (action)
  While, // (condition, action)
  Wait, // (condition)
  If, // (condition, action)
  ElseIf, // (condition, action)
  Else, // (action)
  Do, End, // do ... end
  Add, Sub, Mul, Div, Mod, Neg,
  And, Or, Xor, Not, Choose,
  Gt, Gte, Lt, Lte, Eq, Neq,
  Srand, // (seed)
  Rand, // (modulus)
  Restart, ChipId, FlashChipId, CycleCount,
} opcode_t;

uint8_t* eval(uint8_t* pc, int32_t* value);
