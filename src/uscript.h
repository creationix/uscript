#ifndef USCRIPT_H
#define USCRIPT_H
#include <stdint.h>
#ifdef ARDUINO
  #include "Arduino.h"
  #include "uscript-wiring.h"
#endif

#define MAX_STACK 64

struct state;

typedef void (*function)(struct state *s);

struct state {
  function* fns;
  const char* names;
  intptr_t stack[MAX_STACK];
  intptr_t* top;
  function* pc;
};

#define DONEXT ++s->pc; if (*s->pc) return (*s->pc)(s);

int compile(struct state* S, function* out, int max, const char* in);

void Call(struct state* s);
void Num(struct state* s);
void Add(struct state* s);
void Sub(struct state* s);
void Mul(struct state* s);
void Div(struct state* s);
void Mod(struct state* s);
void Neg(struct state* s);
void Not(struct state* s);
void Swap(struct state* s);
void Over(struct state* s);
void Dup(struct state* s);
void Decr(struct state* s);
void Incr(struct state* s);
void IsTrue(struct state* s);
void IsFalse(struct state* s);
void Jump(struct state* s);
void Random(struct state* s);
void SeedRandom(struct state* s);
void Start(struct state* s, function* code);

#endif
