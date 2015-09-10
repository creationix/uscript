#ifndef USCRIPT_H
#define USCRIPT_H

#include <stdint.h>
#include <stdbool.h>

// These defines control the capability and size of coroutines
#define MAX_INSTRUCTIONS 8 // Max instruction depth per thread
#define MAX_VALUES 8       // Max value depth per thread

// These define the capability and size of VM states.
#define MAX_DEFS 32
#define MAX_COROUTINES 8

typedef enum {
  EMPTY = 128, // internal opcode for consuming arguments.
  DO, DOING, END, // *Run all expressions inside returning last value.
  RETURN, // *(value) return early from DO..END with value.
  YIELD, // Pauses the current thread, putting it at the end of the event queue.
  DELAY, // (ms) Pause, but don't resume till after delay timeout.
  WHILE, // *(cond) {body} repeatedly run body while condition is true.
  WAIT, // *(cond) repeatedly run condition till it's true.
  PM,  // (pin, mode) Set Pin Mode
  DW,  // (pin, value) Digital write to pin
  DR,  // (pin) Digital read from pin
  AW,  // (pin, value) Analog (pwm) write to pin
  AR,  // (pin) Analog read from pin
  NOT, // (val) logical NOT
  AND, // (val, val) logical AND with value preservation and short-circuit
  OR,  // (val, val) logical OR with value preservation and short-circuit
  XOR, // (val, val) logic XOR with value preservation
  NEG, // (val) negate a value
  ADD, // (val, val) Add two values
  SUB, // (val, val) subtract two values
  MUL, // (val, val) multiple two values
  DIV, // (val, val) divide two values
  MOD, // (val, mod) modulus of two values
  GT,  // (val, val) greater than
  LT,  // (val, val) less than
  GTE, // (val, val) greater than or equal
  LTE, // (val, val) less than or equal
  EQ,  // (val, val) equal
  NEQ, // (val, val) not equal
  IF, THEN, ELIF, ELSE, // if (cond) {body} elif (cond) {body} else {body}
  DEF, // *(id, nargs) {body} define an expression
  CALL, // *(id, args...) run expression at target address and pass through value.
  TCALL, // *(id, args...) run expression at target and return value.
} instruction_t;

typedef struct {
  uint8_t istack[MAX_INSTRUCTIONS]; // Stack of instructions
  uint8_t* i; // pointer to current top of instruction stack
  int32_t vstack[MAX_VALUES]; // Stack of values
  int32_t* v; // pointer to current top of value stack
  uint8_t* pc; // pointer to current program counter (next instruction)
  unsigned long again; // startup again after millis() is >= again. (0 disables)
} coroutine_t;

typedef struct {
  int len;
  int nargs;
  uint8_t* code[];
} def_t;

typedef struct {
  coroutine_t coroutines[MAX_COROUTINES];
  def_t defs[MAX_DEFS];
} state_t;

bool skip(state_t* S, coroutine_t* T);
bool fetch(state_t* S, coroutine_t* T);
bool step(state_t* S, coroutine_t* T);
bool loop(state_t* S);
bool coroutine_create(state_t* S, uint8_t* pc);
#endif
