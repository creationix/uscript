#include "uscript.h"
#ifdef ARDUINO
  #include "Arduino.h"
#else
  #include "wiring-polyfill.c"
#endif
#ifdef VERBOSE_VM
#include <assert.h>
#endif

#include <stdio.h>

const char* names[] = {
  "EMPTY", // internal opcode for consuming arguments.
  "DO", "DOING", "END", // Run all expressions inside returning last value.
  "RETURN", // (value) return early from DO..END with value.
  "YIELD", // Pauses the current thread, putting it at the end of the event queue.
  "DELAY", // (ms) Pause, but don't resume till after delay timeout.
  "PM",  // (pin, mode) Set Pin Mode
  "DW",  // (pin, value) Digital write to pin
  "DR",  // (pin) Digital read from pin
  "AW",  // (pin, value) Analog (pwm) write to pin
  "AR",  // (pin) Analog read from pin
  "NOT", // (val) logical NOT
  "AND", // (val, val) logical AND with value preservation and short-circuit
  "OR",  // (val, val) logical OR with value preservation and short-circuit
  "XOR", // (val, val) logic XOR with value preservation
  "NEG", // (val) negate a value
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
  "SRAND", // (seed) seed for PRNG
  "RAND", // (mod) generate new PRNG and modulus the output
  "IF", "THEN", "ELIF", "ELSE", // if (cond) {body} elif (cond) {body} else {body}
  "WHILE", // (cond) {body} repeatedly run body while condition is true.
  "WAIT", // (cond) repeatedly run condition till it's true.
  "DEF", // (id, nargs) {body} define an expression
  "CALL", // (id, args...) run expression at target address and pass through value.
  "TCALL", // (id, args...) run expression at target and return value.
};

// Stack operations
#define push(L, v) (*(++(L)) = (v))
#define pop(L) (*((L)--))
#define peek(L) (*(L))
#define peek1(L) (*((L) + 1))

#define shift(L) *((L)++)

bool skip(state_t* S, coroutine_t* T) {
  if (peek(T->pc) < 128) {
    shift(T->pc);
    // TODO: handle numbers larger than 127
    return true;
  }
  instruction_t op = shift(T->pc);
  switch (op) {
    // 2 arguments
    case ADD: case SUB: case MUL: case DIV: case MOD: case XOR:
    case GT: case LT: case GTE: case LTE: case EQ: case NEQ:
    case AND: case OR: case PM: case DW: case AW:
      return skip(S, T) && skip(S, T);

    // 1 argument
    case NEG: case NOT: case DR: case AR: case DELAY: case SRAND: case RAND:
    case WAIT:
      return skip(S, T);

    // 0 arguments
    case YIELD:
      return true;

    case DO:
      while (peek(T->pc) != END) {
        if (!skip(S, T)) return false;
      }
      shift(T->pc);
      return true;

    case IF:
      // Skip condition and body
      if (!(skip(S, T) && skip(S, T))) return false;

      // Skip any ELIF blocks, if any.
      while (peek(T->pc) == ELIF) {
        shift(T->pc);
        if (!(skip(S, T) && skip(S, T))) return false;
      }

      // Skip ELSE block if there is one.
      if (peek(T->pc) == ELSE) {
        shift(T->pc);
        if (!skip(S, T)) return false;
      }
      return true;

    case EMPTY: case DOING: case RETURN: case END:
    case THEN: case ELIF: case ELSE:
      printf("Unexpected opcode in skip %s\n", names[op - 128]);
      T->pc = 0;
      return false;

    case WHILE:
    case DEF: case CALL: case TCALL:
      printf("TODO: Implement skip for %s\n", names[op - 128]);
      T->pc = 0;
      return false;
  }
  return true;
}

bool fetch(state_t* S, coroutine_t* T) {
  if (peek(T->pc) < 128) {
    int32_t value = shift(T->pc);
    // TODO: handle numbers larger than 127
    #ifdef VERBOSE_VM
    printf("%d\n", value);
    #endif
    push(T->v, value);
    return true;
  }
  instruction_t op = shift(T->pc);
  #ifdef VERBOSE_VM
  printf("%s\n", names[op - 128]);
  #endif
  switch (op) {
    case DO:
      push(T->i, op);
      return true;
    case ADD: case SUB: case MUL: case DIV: case MOD: case XOR:
    case GT: case LT: case GTE: case LTE: case EQ: case NEQ:
    case PM: case DW: case AW:
      push(T->i, op);
      push(T->i, EMPTY);
      return fetch(S, T);
    case NEG: case NOT: case AND: case OR: case IF:
    case DR: case AR: case SRAND: case RAND: case DELAY:
      push(T->i, op);
      return fetch(S, T);
    case YIELD:
      push(T->v, 0);
      return false;
    case WAIT:
      push(T->i, WAIT);
      push(T->r, T->pc);
      return fetch(S, T);
    case EMPTY: case DOING: case THEN: case ELIF: case ELSE: case RETURN:
      printf("Unexpected opcode in fetch %s\n", names[op - 128]);
      T->pc = 0;
      return false;
    case END: case WHILE:
    case DEF: case CALL: case TCALL:
      printf("TODO: Implement fetch for %s\n", names[op - 128]);
      T->pc = 0;
      return false;
  }
  return false;
}

static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;

bool step(state_t* S, coroutine_t* T) {
  #ifdef VERBOSE_VM
  assert(T->v < T->vstack + MAX_VALUES);
  assert(T->i < T->istack + MAX_INSTRUCTIONS);
  printf("iStack(");
  uint8_t* i;
  for (i = T->istack; i <= T->i; i++) {
    printf(" %s", names[*i - 128]);
  }
  printf(" ) vStack(");
  int32_t* v;
  for (v = T->vstack; v <= T->v; v++) {
    printf(" %d", *v);
  }
  printf(" ) rStack(");
  uint8_t** r;
  for (r = T->rstack; r <= T->r; r++) {
    printf(" %p", *r);
  }
  printf(" )\n");
  #endif
  // When the instruction stack runs out, we're done with this coroutine.
  if (T->i < T->istack) {
    T->pc = 0;
    return false;
  }
  instruction_t op = pop(T->i);
  switch(op) {
    case EMPTY: return fetch(S, T);
    case DO:
      // Transform to Doing and start new fetch.
      push(T->i, DOING);
      return fetch(S, T);
    case DOING:
      // If End if reached, let Doing drop and move on.
      if (peek(T->pc) == END) {
        shift(T->pc);
        break;
      }
      // Otherwise preserve Doing and drop last value and start new fetch.
      pop(T->v);
      if (peek(T->pc) == RETURN) {
        push(T->i, shift(T->pc));
        return fetch(S, T);
      }
      T->i++; // un-consume doing instruction that fetched us
      return fetch(S, T);
    case RETURN:
      // Skip to end if we had a return.
      while (peek(T->pc) != END) {
        if (!skip(S, T)) return false;
      }
      shift(T->pc);
      break;
    if (peek(T->pc) == END) {
      shift(T->pc);
      push(T->v, 0);
      break;
    }

    case DELAY:
      T->again = millis() + peek(T->v);
      return false;
    case PM: {
      int32_t mode = pop(T->v);
      pinMode(peek(T->v), mode);
      peek(T->v) = mode;
      break;
    }
    case DW: {
      int32_t value = pop(T->v);
      digitalWrite(peek(T->v), value);
      peek(T->v) = value;
      break;
    }
    case DR:
      peek(T->v) = digitalRead(peek(T->v));
      break;
    case AW: {
      int32_t value = pop(T->v);
      analogWrite(peek(T->v), value);
      peek(T->v) = value;
      break;
    }
    case AR:
      peek(T->v) =  analogRead(peek(T->v));
      break;
    case NOT: peek(T->v) = !peek(T->v); break;
    case AND:
      // If first value is truthy, drop it and capture second value.
      if (peek(T->v)) {
        pop(T->v);
        return fetch(S, T);
      }
      // Otherwise keep false and skip second value.
      return skip(S, T);
    case OR:
      // If the first value was truthy, keep it and skip second value.
      if (peek(T->v)) return skip(S, T);
      // Otherwise, throw away first value and capture second.
      pop(T->v);
      return fetch(S, T);
    case XOR: {
      int32_t b = pop(T->v),
              a = peek(T->v);
      peek(T->v) = a ? (b ? 0 : a) : (b ? b : 0);
      break;
    }
    case NEG: peek(T->v) = -peek(T->v); break;
    case ADD: pop(T->v); peek(T->v) += peek1(T->v); break;
    case SUB: pop(T->v); peek(T->v) -= peek1(T->v); break;
    case MUL: pop(T->v); peek(T->v) *= peek1(T->v); break;
    case DIV: pop(T->v); peek(T->v) /= peek1(T->v); break;
    case MOD: pop(T->v); peek(T->v) %= peek1(T->v); break;
    case GT:  pop(T->v); peek(T->v) = peek(T->v) > peek1(T->v); break;
    case LT:  pop(T->v); peek(T->v) = peek(T->v) < peek1(T->v); break;
    case GTE: pop(T->v); peek(T->v) = peek(T->v) >= peek1(T->v); break;
    case LTE: pop(T->v); peek(T->v) = peek(T->v) <= peek1(T->v); break;
    case EQ:  pop(T->v); peek(T->v) = peek(T->v) == peek1(T->v); break;
    case NEQ: pop(T->v); peek(T->v) = peek(T->v) != peek1(T->v); break;
    case SRAND:
      deadbeef_seed = peek(T->v);
      deadbeef_beef = 0xdeadbeef;
      break;
    case RAND:
      // From http://inglorion.net/software/deadbeef_rand/
      deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
      deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
      peek(T->v) = deadbeef_seed % peek(T->v);
      break;
    case IF:
      // When condition is true, setup Then and capture value
      if (pop(T->v)) {
        push(T->i, THEN);
        return fetch(S, T);
      }
      // Otherwise, skip the body.
      if (!skip(S, T)) return false;

      // If there is an ELIF, transform it to an IF and start over.
      if (peek(T->pc) == ELIF) {
        shift(T->pc);
        push(T->i, IF);
        return fetch(S, T);
      }

      // If there is an ELSE, start next fetch to capture it.
      if (peek(T->pc) == ELSE) {
        shift(T->pc);
        return fetch(S, T);
      }

      // If nothing matched, push false
      push(T->v, 0);
      break;
    case THEN:
      // Skip any ELIF blocks, if any.
      while (peek(T->pc) == ELIF) {
        shift(T->pc);
        if (!(skip(S, T) && skip(S, T))) return false;
      }

      // Skip ELSE block if there is one.
      if (peek(T->pc) == ELSE) {
        shift(T->pc);
        if (!skip(S, T)) return false;
      }
      break;
    case WAIT:
      if (!peek(T->v)) {
        pop(T->v);
        T->pc = peek(T->r);
        push(T->i, WAIT);
        fetch(S, T);
        return false;
      }
      pop(T->r);
      break;
    case END: case WHILE: case YIELD:
    case ELIF: case ELSE:
    case DEF: case CALL: case TCALL:
      printf("TODO: Implement step for %s\n", names[op - 128]);
      T->pc = 0;
      return false;
  }
  return true;
}

coroutine_t* coroutine_create(state_t* S, uint8_t* pc) {
  int i;
  for (i = 0; S->coroutines[i].pc; i++) {
    if (i >= MAX_COROUTINES) return 0;
  }
  S->coroutines[i].istack[0] = EMPTY;
  // i points to first item, the empty.
  S->coroutines[i].i = S->coroutines[i].istack;
  // v points to space above stack since it's empty.
  S->coroutines[i].v = S->coroutines[i].vstack - 1;
  S->coroutines[i].r = S->coroutines[i].rstack - 1;
  S->coroutines[i].pc = pc;
  S->coroutines[i].again = 0;
  return S->coroutines + i;
}

bool loop(state_t* S, unsigned long timeout) {
  unsigned long now = millis();
  bool more = false;

  // Calculate time to first live coroutine
  int i;
  for (i = 0; i < MAX_COROUTINES; i++) {
    coroutine_t* T = S->coroutines + i;
    if (!T->pc) continue;
    more = true;
    if (!timeout || T->again <= now) {
      timeout = 0;
    }
    else {
      unsigned long delta = T->again - now;
      timeout = timeout < delta ? timeout : delta;
    }
  }
  if (!more) return false;

  if (timeout) {
    delay(timeout);
    now = millis();
  }

  more = false;
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
    #ifdef VERBOSE_VM
    printf("Running %td\n", T - S->coroutines);
    #endif
    while (step(S, T));

    more = more || T->pc;
  }
  return more;
}
