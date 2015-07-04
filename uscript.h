#ifndef USCRIPT_H
#define USCRIPT_H

#include <stdint.h>
#include <string.h>

// #include "rpi-io.c"

#if defined(SPARK)
  #define BUFFER_LENGTH 512
  #include "application.h"
  #define number int32_t
  #define OP_WIRING
  #define assert(x)
#elif defined(ARDUINO)
  #define BUFFER_LENGTH 512
  #include "Arduino.h"
  #define number int32_t
  #define OP_WIRING
  #define assert(x)
#else
  #define BUFFER_LENGTH 4096
  #include <assert.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <inttypes.h>
  #define number int64_t
  #ifdef BCM2708_PERI_BASE
    #define OP_WIRING
  #endif
#endif

typedef void (*write_string_fn)(const char* str);
typedef void (*write_char_fn)(char c);
typedef void (*write_number_fn)(number num);

struct state {
  write_string_fn write_string;
  write_char_fn write_char;
  write_number_fn write_number;
  number vars[26];
  uint8_t* stubs[26];
};

int compile(uint8_t* program);
uint8_t* skip(uint8_t* pc);
uint8_t* eval(struct state* vm, uint8_t* pc, number* res);
void handle_input(struct state* vm, char c);
void start_state(struct state* vm);

#endif
