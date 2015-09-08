#include "uscript.h"
#include <stdio.h>

const char* names[] = {
  "EMPTY", // internal opcode for consuming arguments.
  "DO", "DOING", "END", // Run all expressions inside returning last value.
  "RETURN", // (value) return early from DO..END with value.
  "YIELD", // Pauses the current thread, putting it at the end of the event queue.
  "DELAY", // (ms) Pause, but don't resume till after delay timeout.
  "WHILE", // (cond) {body} repeatedly run body while condition is true.
  "WAIT", // (cond) repeatedly run condition till it's true.
  "PM",  // (pin, mode) Set Pin Mode
  "DW",  // (pin, value) Digital write to pin
  "DR",  // (pin) Digital read from pin
  "AW",  // (pin, value) Analog (pwm) write to pin
  "AR",  // (pin) Analog read from pin
  "NOT", // (val) logical NOT
  "AND", // (val, val) logical AND with value preservation and short-circuit
  "OR",  // (val, val) logical OR with value preservation and short-circuit
  "XOR", // (val, val) logic XOR with value preservation
  "ADD", // (val, val) Add two values
  "SUB", // (val, val) subtract two values
  "MUL", // (val, val) multiple two values
  "DIV", // (val, val) divide two values
  "MOD", // (val, mod) modulus of two values
  "GT",  // (val, val) greater than
  "LT",  // (val, val) less than
  "GTE", // (val, val) greater than or equal
  "LTE", // (val, val) less than or equal
  "EQ",  // (val, val) equal
  "NEQ", // (val, val) not equal
  "IF", "THEN", "ELIF", "ELSE", // if (cond) {body} elif (cond) {body} else {body}
  "DEF", // (id, nargs) {body} define an expression
  "CALL", // (id, args...) run expression at target address and pass through value.
  "TCALL", // (id, args...) run expression at target and return value.
};

extern unsigned long millis();

bool skip(state_t* S, coroutine_t* T) {
  return false;
}

bool fetch(state_t* S, coroutine_t* T) {
  if (*T->pc < 128) {
    printf("%d\n", *T->pc);
    *T->v++ = *T->pc++;
    // TODO: handle numbers larger than 127
    return true;
  }
  printf("%s\n", names[*T->pc - 128]);
  switch((instruction_t)*T->pc) {
    case ADD: case SUB: case MUL: case DIV: case MOD:
      *(T->i)++ = *T->pc++;
      *(T->i)++ = EMPTY;
      *(T->i)++ = EMPTY;
      return true;
    case DELAY:
      T->i++;
      T->pc++;
      *(T->i)++ = DELAY;
      *(T->i)++ = EMPTY;
      return true;
    case YIELD:
      T->i++;
      T->pc++;
      return false;
    default:
      printf("TODO: Implement fetch for %s\n", names[T->i[0] - 128]);
      T->pc = 0;
      return false;
  }
  return false;
}

bool step(state_t* S, coroutine_t* T) {
  printf("iStack:");
  uint8_t* i;
  for (i = T->istack; i < T->i; i++) {
    printf(" %s", names[*i - 128]);
  }
  printf("\nvStack:");
  int32_t* v;
  for (v = T->vstack; v < T->v; v++) {
    printf(" %d", *v);
  }
  printf("\n");

  // When the instruction stack runs out, we're done with this coroutine.
  if (T->i <= T->istack) {
    T->pc = 0;
    return false;
  }
  switch((instruction_t)*--T->i) {
    case EMPTY: return fetch(S, T);
    case DELAY: {
      int delay = *--T->v;
      T->again = millis() + delay;
      return false;
    }
    case ADD: T->v--; *(T->v - 1) += *T->v; break;
    case SUB: T->v--; *(T->v - 1) -= *T->v; break;
    case MUL: T->v--; *(T->v - 1) *= *T->v; break;
    case DIV: T->v--; *(T->v - 1) /= *T->v; break;
    case MOD: T->v--; *(T->v - 1) %= *T->v; break;
    default:
      printf("TODO: Implement step for %s\n", names[T->i[0] - 128]);
      T->pc = 0;
      return false;
  }
  return true;
}

bool loop(state_t* S) {
  unsigned long now = millis();
  bool more = false;
  int i;
  for (i = 0; i < MAX_COROUTINES; i++) {
    coroutine_t* T = S->coroutines + i;

    // Skip unused coroutines
    if (!T->pc) continue;

    // Skip coroutines that are waiting for a delay
    if (T->again > now) {
      more = true;
      continue;
    }

    // Run till stop
    printf("Running %p\n", T);
    while (step(S, T));

    more = more || T->pc;
  }
  return more;
}

bool coroutine_create(state_t* S, uint8_t* pc) {
  int i;
  for (i = 0; S->coroutines[i].pc; i++) {
    if (i >= MAX_COROUTINES) return false;
  }
  S->coroutines[i].istack[0] = EMPTY;
  S->coroutines[i].i = S->coroutines[i].istack + 1;
  S->coroutines[i].v = S->coroutines[i].vstack;
  S->coroutines[i].r = S->coroutines[i].rstack;
  S->coroutines[i].pc = pc;
  S->coroutines[i].again = 0;
  return true;
}
