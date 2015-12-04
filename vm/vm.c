#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

typedef enum {
  Mode = 128, // (pin, mode)
  Write, // (pin, value)
  Delay, // (ms)
  Forever, // (action)
  Do, End, // do ... end
} opcode_t;

static void setMode(int pin, int mode) {
  printf("setMode(%d, %d)\n", pin, mode);
}

static void digitalWrite(int pin, int value) {
  printf("digitalWrite(%d, %d)\n", pin, value);
}

static void delay(int ms) {
  struct timeval t;
  t.tv_sec = ms / 1000;
  t.tv_usec = (ms % 1000) * 1000;
  select(0, NULL, NULL, NULL, &t);
}

// 0mxxxxxx mxxxxxxx* - integer
// 1xxxxxxx - opcode

uint8_t* eval(uint8_t* pc, int32_t* value) {
  if (!(*pc & 0x80)) {
    *value = *pc & 0x3f;
    if (*pc++ & 0x40) {
      do {
        *value <<= 7;
        *value |= *pc & 0x7f;
      } while (*pc++ & 0x80);
    }
    return pc;
  }
  switch ((opcode_t) *pc++) {
    case Mode: {
      int32_t pin;
      pc = eval(pc, &pin);
      pc = eval(pc, value);
      setMode(pin, *value);
      return pc;
    }
    case Write: {
      int32_t pin;
      pc = eval(pc, &pin);
      pc = eval(pc, value);
      digitalWrite(pin, *value);
      return pc;
    }
    case Delay:
      pc = eval(pc, value);
      delay(*value);
      return pc;
    case Forever: {
      uint8_t* start = pc;
      // TODO: add way to exit loop.
      while (1) {
        pc = eval(start, value);
      }
      return pc;
    }
    case Do:
      while (*pc != End) {
        pc = eval(pc, value);
      }
      return pc;
    case End:
      *value = -1;
      return 0;
  }
  return pc;

}

int main() {
  uint8_t code[] = {
  Do,
    Mode, 13, 1,
    Forever, Do,
      Write, 13, 1,
      Delay, 0x47, 0x68,
      Write, 13, 0,
      Delay, 0x47, 0x68,
    End,
  End
  };

  int32_t result;
  uint8_t* pc = eval(code, &result);
  return *pc == (uint8_t)EOF;
}
