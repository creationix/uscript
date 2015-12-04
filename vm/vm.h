typedef enum {
  Mode = 128, // (pin, mode)
  Write, // (pin, value)
  Delay, // (ms)
  Forever, // (action)
  Do, End, // do ... end
} opcode_t;

uint8_t* eval(uint8_t* pc, int32_t* value);

