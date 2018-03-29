#include "vm.h"
#include <stdlib.h>
#include <string.h>

extern int isAlive();

// Nodemcu pin remapping.
static char pins[] = { 16, 5, 4, 0, 2, 14, 12, 13, 15, 3, 1, 9, 10 };
// Normal pin mapping
// static char pins[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

#ifdef ARDUINO
#include "Arduino.h"
#include "Wire.h"

static void printNumber(intptr_t num) {
  Serial.println(num, DEC);
}

#else
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

static int pinState[17];

static void pinMode(int pin, int mode) {
  printf("pinMode(%d, %d)\n", pin, mode);
}

static void digitalWrite(int pin, int value) {
  pinState[pin] = !!value;
  printf("digitalWrite(%d, %d)\n", pin, value);
}

static int digitalRead(int pin) {
  printf("digitalRead(%d)\n", pin);
  return !!pinState[pin];
}

static void analogWrite(int pin, int value) {
  pinState[pin] = value;
  printf("analogWrite(%d, %d)\n", pin, value);
}

static int analogRead(int pin) {
  printf("analogRead(%d)\n", pin);
  return pinState[pin];
}

static void delay(int ms) {
  struct timeval t;
  t.tv_sec = ms / 1000;
  t.tv_usec = (ms % 1000) * 1000;
  select(0, NULL, NULL, NULL, &t);
}

static void delayMicroseconds(int ms) {
  struct timeval t;
  t.tv_sec = ms / 1000000;
  t.tv_usec = (ms % 1000000);
  select(0, NULL, NULL, NULL, &t);
}


class FakeESP {
public:
  void restart() {
    printf("ESP.restart()\n");
  }
  int getChipId() {
    printf("ESP.getChipId()\n");
    return 0;
  }
  int getFlashChipId() {
    printf("ESP.getFlashChipId()\n");
    return 0;
  }
  int getCycleCount() {
    printf("ESP.getCycleCount()\n");
    return 0;
  }
  int getFreeHeap() {
    printf("ESP.getFreeHeap()\n");
    return 0;
  }
};
FakeESP ESP;
class FakeWire {
public:
  void begin(int sda, int scl) {
    printf("begin(%d, %d)\n", sda, scl);
  }
  int requestFrom(int address, int quantity, int stop) {
    printf("requestFrom(%d, %d, %d)\n", address, quantity, stop);
    return 0;
  }
  void beginTransmission(int address) {
    printf("beginTransmission(%d)\n", address);
  }
  int endTransmission(int stop) {
    printf("endTransmission(%d)\n", stop);
    return 0;
  }
  int write(int byte) {
    printf("write(%d)\n", byte);
    return 0;
  }
  int available() {
    printf("available()\n");
    return 0;
  }
  int read() {
    printf("read()\n");
    return 0;
  }
};
FakeWire Wire;

static void printNumber(intptr_t num) {
  printf("%ld\n", num);
}

#endif

// 0mxxxxxx mxxxxxxx* - integer
// 1xxxxxxx - opcode

static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;

static intptr_t globals[100];

uint8_t* eval(intptr_t* stack, uint8_t* pc, intptr_t* value) {
  // printf("%s> stack: %p  pc: %p  op: %02x\n", value ? "run" : "skip", stack, pc, *pc);
  if (!(*pc & 0x80)) {
    if (!value) {
      if (*pc++ & 0x40) while (*pc++ & 0x80);
      return pc;
    }
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
    case Dump:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      printNumber(*value);
      return pc;
    case Mode: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t pin;
      pc = eval(stack, pc, &pin);
      pc = eval(stack, pc, value);
      pinMode(pins[pin], *value);
      return pc;
    }
    case Read:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = digitalRead(pins[*value]);
      return pc;
    case Write: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t pin;
      pc = eval(stack, pc, &pin);
      pc = eval(stack, pc, value);
      digitalWrite(pins[pin], *value);
      return pc;
    }
    case Aread:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = analogRead(pins[*value]);
      return pc;
    case Pwrite: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t pin;
      pc = eval(stack, pc, &pin);
      pc = eval(stack, pc, value);
      analogWrite(pins[pin], *value);
      return pc;
    }
    case Ibegin: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t sda;
      pc = eval(stack, pc, &sda);
      pc = eval(stack, pc, value);
      Wire.begin(pins[sda], pins[*value]);
      return pc;
    }
    case Ifrom: {
      if (!value) return eval(stack, eval(stack, eval(stack, pc, 0), 0), 0);
      intptr_t address, quantity, stop;
      pc = eval(stack, pc, &address);
      pc = eval(stack, pc, &quantity);
      pc = eval(stack, pc, &stop);
      *value = Wire.requestFrom(address, quantity, stop);
      return pc;
    }
    case Istart:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      Wire.beginTransmission(*value);
      return pc;
    case Istop:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = Wire.endTransmission(*value);
      return pc;
    case Iwrite:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = Wire.write(*value);
      return pc;
    case Iavailable:
      if (value) *value = Wire.available();
      return pc;
    case Iread:
      if (value) *value = Wire.read();
      return pc;
    case Tone: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t pin, duration;
      pc = eval(stack, pc, &pin);
      pc = eval(stack, pc, value);
      pc = eval(stack, pc, &duration);
      pin = pins[pin];

      int p = 1000000 / *value;
      int d = p >> 1;
      duration *= 1000;
      while ((duration -= p) > 0) {
        digitalWrite(pin, 1);
        delayMicroseconds(d);
        digitalWrite(pin, 0);
        delayMicroseconds(d);
      }
      return pc;

      return pc;
    }
    case Delay:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      delay(*value);
      return pc;
    case Call: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t offset;
      pc = eval(stack, pc, &offset);
      int16_t jmp = (*pc << 8) | *(pc + 1);
      pc += 2;
      eval(stack + offset, pc + jmp, value);
      return pc;
    }
    case Gosub: {
      if (!value) return eval(stack, pc, 0);
      int16_t jmp = (*pc << 8) | *(pc + 1);
      pc += 2;
      eval(stack, pc + jmp, value);
      return pc;
    }
    case Goto: {
      if (!value) return eval(stack, pc, 0);
      int16_t jmp = (*pc << 8) | *(pc + 1);
      pc += 2;
      return eval(stack, pc + jmp, value);
    }
    case Gget:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = globals[*value];
      return pc;
    case Gset: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      globals[num] = *value;
      return pc;
    }
    case Get:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = stack[*value];
      return pc;
    case Set: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      stack[num] = *value;
      return pc;
    }
    case Incr:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = ++stack[*value];
      return pc;
    case Decr:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = --stack[*value];
      return pc;
    case IncrMod: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t index;
      pc = eval(stack, pc, &index);
      pc = eval(stack, pc, value);
      *value = stack[index] = (stack[index] + 1) % *value;
      return pc;
    }
    case DecrMod: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t index;
      pc = eval(stack, pc, &index);
      pc = eval(stack, pc, value);
      *value = stack[index] = (stack[index] + *value - 1) % *value;
      return pc;
    }
    if (!value) return eval(stack, pc, 0);
    case Forever: {
      if (!value) return eval(stack, pc, 0);
      uint8_t* start = pc;
      do {
        pc = eval(stack, start, value);
      } while (isAlive());
      return pc;
    }
    case While: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      uint8_t* start = pc;
      intptr_t cond;
      *value = 0;
      do {
        pc = eval(stack, start, &cond);
        if (!cond) {
          pc = eval(stack, pc, 0);
          break;
        }
        pc = eval(stack, pc, value);
      } while (isAlive());
      return pc;
    }
    case Wait: {
      if (!value) return eval(stack, pc, 0);
      uint8_t* cond = pc;
      do {
        pc = eval(stack, cond, value);
      } while (isAlive() && !*value);
      return pc;
    }
    case If: {
      if (value) goto start;

      pc = eval(stack, pc, 0);
      skip:
      while (*pc == ElseIf) pc = eval(stack, eval(stack, ++pc, 0), 0);
      if (*pc == Else) pc = eval(stack, ++pc, 0);
      return pc;

      start:
      pc = eval(stack, pc, value);
      if (*value) {
        pc = eval(stack, pc, value);
        goto skip;
      }
      pc = eval(stack, pc, 0);
      if (*pc == ElseIf) {
        pc++;
        goto start;
      }
      if (*pc == Else) {
        return eval(stack, ++pc, value);
      }
      return pc;
    }
    case Do: {
      int skip = 0;
      while (*pc != End) {
        if (*pc == Return) {
          skip = 1;
          pc++;
          if (*pc == End) return pc + 1;
        }
        pc = eval(stack, pc, skip ? 0 : value);
      }
      return pc + 1;
    }
    case Add: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num + *value;
      return pc;
    }
    case Sub: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num - *value;
      return pc;
    }
    case Mul: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num * *value;
      return pc;
    }
    case Div: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num / *value;
      return pc;
    }
    case Mod: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num % *value;
      return pc;
    }
    case Neg:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = -(*value);
      return pc;

    case Band: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num & *value;
      return pc;
    }
    case Bor: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num | *value;
      return pc;
    }
    case Bxor: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num ^ *value;
      return pc;
    }
    case Bnot:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = ~*value;
      return pc;
    case Lshift: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = (uint32_t)num << *value;
      return pc;
    }
    case Rshift: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = (uint32_t)num >> *value;
      return pc;
    }
    case And: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      return eval(stack, eval(stack, pc, value), *value ? value : 0);
    }
    case Or: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      return eval(stack, eval(stack, pc, value), *value ? 0 : value);
    }
    case Xor: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num ?
        (*value ? 0 : num) :
        (*value ? *value : 0);
      return pc;
    }
    case Not:
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      *value = !(*value);
      return pc;
    case Choose:
      if (!value) return eval(stack, eval(stack, eval(stack, pc, 0), 0), 0);
      pc = eval(stack, pc, value);
      if (value) return eval(stack, eval(stack, pc, value), 0);
      return eval(stack, eval(stack, pc, 0), value);
    case Gt: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num > *value;
      return pc;
    }
    case Gte: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num >= *value;
      return pc;
    }
    case Lt: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num < *value;
      return pc;
    }
    case Lte: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num <= *value;
      return pc;
    }
    case Eq: {
      if (!value) return eval(stack, eval(stack, pc, 0), 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num == *value;
      return pc;
    }
    case Neq: {
      if (!value) return eval(stack, pc, 0);
      intptr_t num;
      pc = eval(stack, pc, &num);
      pc = eval(stack, pc, value);
      *value = num != *value;
      return pc;
    }
    case Srand: {
      if (!value) return eval(stack, pc, 0);
      pc = eval(stack, pc, value);
      deadbeef_seed = *value;
      deadbeef_beef = 0xdeadbeef;
      return pc;
    }
    case Rand: {
      if (!value) return eval(stack, pc, 0);
      // From http://inglorion.net/software/deadbeef_rand/
      deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
      deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
      pc = eval(stack, pc, value);
      *value = deadbeef_seed % *value;
      return pc;
    }
    case Restart:
      if (value) ESP.restart();
      return pc;
    case ChipId:
      if (value) *value = ESP.getChipId();
      return pc;
    case FlashChipId:
      if (value) *value = ESP.getFlashChipId();
      return pc;
    case CycleCount:
      if (value) *value = ESP.getCycleCount();
      return pc;
    case GetFree:
      if (value) *value = ESP.getFreeHeap();
      return pc;

    // Invalid cases (cannot start an expression)
    case End: case ElseIf: case Else: case Return:
      *value = -(*(pc - 1));
      return 0;

  }
  return pc;

}