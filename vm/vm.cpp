#include "vm.h"

extern int isAlive();

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

static void pinMode(int pin, int mode) {
  printf("pinMode(%d, %d)\n", pin, mode);
}

static void digitalWrite(int pin, int value) {
  printf("digitalWrite(%d, %d)\n", pin, value);
}

static int digitalRead(int pin) {
  printf("digitalRead(%d)\n", pin);
  return 0;
}

static void analogWrite(int pin, int value) {
  printf("analogWrite(%d, %d)\n", pin, value);
}

static int analogRead(int pin) {
  printf("analogRead(%d)\n", pin);
  return 0;
}


static void delay(int ms) {
  struct timeval t;
  t.tv_sec = ms / 1000;
  t.tv_usec = (ms % 1000) * 1000;
  select(0, NULL, NULL, NULL, &t);
}
#endif

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
      pinMode(pin, *value);
      return pc;
    }
    case Read:
      pc = eval(pc, value);
      *value = digitalRead(*value);
      return pc;
    case Write: {
      int32_t pin;
      pc = eval(pc, &pin);
      pc = eval(pc, value);
      digitalWrite(pin, *value);
      return pc;
    }
    case Aread:
      pc = eval(pc, value);
      *value = analogRead(*value);
      return pc;
    case Pwrite: {
      int32_t pin;
      pc = eval(pc, &pin);
      pc = eval(pc, value);
      analogWrite(pin, *value);
      return pc;
    }
    case Delay:
      pc = eval(pc, value);
      delay(*value);
      return pc;
    case Forever: {
      uint8_t* start = pc;
      // TODO: add way to exit loop.
      while (isAlive()) {
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
    case Add: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num + *value;
      return pc;
    }
    case Sub: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num - *value;
      return pc;
    }
    case Mul: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num * *value;
      return pc;
    }
    case Div: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num / *value;
      return pc;
    }
    case Mod: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num % *value;
      return pc;
    }
    case Neg:
      pc = eval(pc, value);
      *value = -(*value);
      return pc;
    case And: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num && *value;
      return pc;
    }
    case Or: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num || *value;
      return pc;
    }
    case Xor: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = (num && !*value) || (!num && *value);
      return pc;
    }
    case Not:
      pc = eval(pc, value);
      *value = !(*value);
      return pc;
    case Gt: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num > *value;
      return pc;
    }
    case Gte: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num >= *value;
      return pc;
    }
    case Lt: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num < *value;
      return pc;
    }
    case Lte: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num <= *value;
      return pc;
    }
    case Eq: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num == *value;
      return pc;
    }
    case Neq: {
      int32_t num;
      pc = eval(pc, &num);
      pc = eval(pc, value);
      *value = num != *value;
      return pc;
    }
  }
  return pc;

}
/*
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
*/
