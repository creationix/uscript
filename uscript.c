#include "uscript.h"

extern unsigned long millis();

bool skip(state_t* S, coroutine_t* T) {
  return false;
}

bool fetch(state_t* S, coroutine_t* T) {
  return false;
}

bool step(state_t* S, coroutine_t* T) {
  return false;
}

bool loop(state_t* S) {
  unsigned long now = millis();
  int i;
  for (i = 0; i < MAX_COROUTINES; i++) {
    coroutine_t* T = S->coroutines + i;
    if (T->cond) {
      // TODO: evaluate condition
    }
    else if (T->again < now) {
      while (step(S, T));
    }
  }
  return false;

}
