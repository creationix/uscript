#ifdef PLATFORM_INCLUDES
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

// Assume unix on arm is raspberry pi for now
// We need much better logic in the long run.
#ifdef __arm__
#include "rpi-io.c"
#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define number uint64_t
#define DMAX 16
#define RMAX 16

#endif

#ifndef PLATFORM_UNIX
#define PLATFORM_UNIX
#include "f2.c"

// unsigned char code[] = {
//   0x2a, // 42
//   0x3a, // 58
//   0x40, 0x64, // 100
//   OP_ADD,
//   OP_ADD,
//   0x32, // 50
//   OP_SUB,
//   4, OP_WHILE, // Loop while not zero
//   10,
// };
// int code_len = 11;

//unsigned char code[] = {
//  0x3f, // i = 63
//  0x3f, OP_OVER, OP_SUB, OP_PRINT, // PRINT 63 - i
//  0x5e, 0xc2, 0x20, OP_DELAY, // DELAY 500,000
//  OP_DECR, // i--
//  11, OP_WHILE,
//};
//int code_len = 12;

unsigned char code[] = {
  16, 1, OP_PM,
  17, 1, OP_PM,
  18, 1, OP_PM,
  19, 1, OP_PM,
  20, 1, OP_PM,
  21, 1, OP_PM,
  22, 1, OP_PM,
  23, 1, OP_PM,
  24, 1, OP_PM,
  25, 1, OP_PM,
  16, 1, OP_DW,
  17, 1, OP_DW,
  18, 1, OP_DW,
  19, 1, OP_DW,
  20, 1, OP_DW,
  21, 1, OP_DW,
  22, 1, OP_DW,
  23, 1, OP_DW,
  24, 1, OP_DW,
  25, 1, OP_DW,
};
int code_len = 60;

// unsigned char code[] = {
//   0, // 0
//   0x43, 0xdc, 0xeb, 0x94, 0x00,  // 1000000000
//   OP_SAVE, // Save a copy of count
//   OP_ADD, // Add count into sum
//   OP_GET, // Restore count
//   OP_DECR, // Decrement count
//   6, OP_WHILE, // loop while count is non-zero
//   OP_POP, // pop the 0 count leaving the sum
// };
// int code_len = 13;

int main() {

  #ifdef BCM2708_PERI_BASE
  setup_io();
  #endif
  d = dstack - 1;
  r = rstack - 1;

  pc = code;
  end = code + code_len;
  const char* err = eval();

  dump(code);

  if (err) {
    printf("ERROR: %s\n", err);
    return -1;
  }
  return 0;
}

#endif

#ifdef PLATFORM_OPCODES
  OP_PRINT, OP_DELAY,
  #ifdef BCM2708_PERI_BASE
    OP_PM, OP_DW, OP_DR, OP_TONE,
  #endif
#endif

#ifdef PLATFORM_OPNAMES
  "PRINT\0DELAY\0"
  #ifdef BCM2708_PERI_BASE
  "PM\0DW\0DR\0TONE\0"
  #endif
#endif

#ifdef PLATFORM_CASES
  case OP_PRINT:
    printf(KBLU"%"PRId64 KNRM"\n", *d--);
    break;
  case OP_DELAY: {
    struct timeval t;
    t.tv_sec = *d / 1000000;
    t.tv_usec = *d % 1000000;
    select(0, NULL, NULL, NULL, &t);
    --d;
    break;
  }
  #ifdef BCM2708_PERI_BASE
    case OP_PM:
      if (*d--) {
        INP_GPIO(*d);
        OUT_GPIO(*d);
      }
      else {
        INP_GPIO(*d);
      }
      --d;
      break;
    case OP_DW:
      if (*d--) GPIO_SET = 1 << *d--;
      else GPIO_CLR = 1 << *d--;
      break;
    case OP_DR:
      *d = !!GET_GPIO(*d);
      break;
    case OP_TONE: {
      number dur = *d--;
      struct timeval t;
      t.tv_sec = 0;
      int p = 1000000 / *d--;
      t.tv_usec = p >> 1;
      number pin = 1 << *d--;
      while ((dur -= p) > 0) {
        GPIO_SET = pin;
        select(0, NULL, NULL, NULL, &t);
        GPIO_CLR = pin;
        select(0, NULL, NULL, NULL, &t);
      }
      break;
    }
  #endif
#endif
