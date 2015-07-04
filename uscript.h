#ifndef USCRIPT_H
#define USCRIPT_H

#include <stdint.h>
#include <string.h>

#if defined(SPARK)
  #define REPL_BUFFER 512
  #include "application.h"
  typedef int32_t number;
  #define OP_WIRING
  #define assert(x)
#elif defined(ARDUINO)
  #define REPL_BUFFER 512
  #if ARDUINO >= 100
    #include "Arduino.h"
  #endif
  typedef int32_t number;
  #define OP_WIRING
  #define assert(x)
#else
  #define REPL_BUFFER 4096
  #include <assert.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <inttypes.h>
  typedef int64_t number;
  #ifdef BCM2708_PERI_BASE
    #define OP_WIRING
  #endif
#endif

typedef void (*write_string_fn)(const char* str);
typedef void (*write_char_fn)(char c);
typedef void (*write_number_fn)(number num);
typedef int (*idle_fn)();

struct state {
  write_string_fn write_string;
  write_char_fn write_char;
  write_number_fn write_number;
  idle_fn idle;
  number vars[26];
  uint8_t* stubs[26];
};

const char* op_to_name(int op);
int name_to_op(const char* name, int len);

uint32_t deadbeef_rand();
void deadbeef_srand(uint32_t x);

int compile(uint8_t* program);
uint8_t* skip(uint8_t* pc);
uint8_t* eval(struct state* vm, uint8_t* pc, number* res);
void handle_input(struct state* vm, char c);
void start_state(struct state* vm);

#endif
