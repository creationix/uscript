#include "uscript.h"
void printStr(const char* str);
void printNum(int str);


static function nameToFunction(struct state* S, const char* name, int len) {
  // First search in the language-defined names.
  const char* list = S->names;
  int op = 0;
  while (*list) {
    int i;
    for (i = 0; *list == name[i]; list++) {
      if (++i == len && !*(list + 1)) return S->fns[op];
    }
    while (*list++);
    op++;
  }
  return 0;
}
const char* functionToName(struct state* S, function fn) {
  const char* list = S->names;
  int op = 0;
  while (*list) {
    if (S->fns[op] == fn) return list;
    while (*list++);
    op++;
  }
  return 0;
}

int compile(struct state* S, function* out, int max, const char* in) {
  int wasJump = 0;
  const char* cc = in;
  function* end = out + max;
  while (out < end) {
    // Exit on null byte in input
    if (!*cc) {
      *out = 0;
      return -1;
    }
    // Skip whitespace
    if (*cc == ' ' || *cc == '\r' || *cc == '\n' | *cc == '\t') {
      cc++;
      continue;
    }

    // Consume keywords
    if (*cc >= 'A' && *cc <= 'Z') {
      int len = 1;
      while (*(cc + len) >= 'A' && *(cc + len) <= 'Z' ) len++;
      function fn = nameToFunction(S, cc, len);
      if (!fn) return cc - in;
      *out++ = fn;
      wasJump = fn == Jump;
      // printf("%.*s\n", len, cc);
      cc += len;
      continue;
    }

    #ifdef ARDUINO
    // Consume constants
    if (*cc == 'a' && *(cc + 1) >= '0' && *(cc + 1) <= '9') {
      cc++;
      intptr_t val = *cc - '0' + A0;
      cc++;
      // printf("NUM %td\n", val);
      *out++ = Num;
      *out++ = (function)val;
      continue;
    }
    #endif

    // Consume integers
    if ((*cc >= '0' && *cc <= '9') || *cc == '-') {
      int neg = 1;
      if (*cc == '-') {
        neg = -1;
        cc++;
      }
      intptr_t val = 0;
      if (*cc == '0' && (*(cc + 1) == 'x' || *(cc + 1) == 'X')) {
        cc += 2;
        // Parse the decimal ascii int
        while (1) {
          if (*cc >= '0' && *cc <= '9') {
            val = (val << 4) | (*cc++ - '0');
          }
          else if (*cc >= 'a' && *cc <= 'f') {
            val = (val << 4) | (*cc++ - ('a' - 10));
          }
          else if (*cc >= 'A' && *cc <= 'F') {
            val = (val << 4) | (*cc++ - ('A' - 10));
          }
          else break;
        }
      }
      else {
        // Parse the decimal ascii int
        do {
          val = val * 10 + *cc++ - '0';
        } while (*cc >= '0' && *cc <= '9');
      }
      val *= neg;
      // printf("NUM %td\n", val);
      if (!wasJump) {
        *out++ = Num;
      }
      wasJump = 0;
      *out++ = (function)val;
      continue;
    }
    break;
  }
  return cc - in;
}


void Call(struct state* s) {
  s->pc++;
  function* ret = s->pc;
  s->pc += (intptr_t)*s->pc;
  s->pc++;
  if (*s->pc) (*s->pc)(s);
  s->pc = ret;
  DONEXT
}
void Num(struct state* s) {
  s->pc++;
  *++s->top = (intptr_t)*s->pc;
  DONEXT
}
void Add(struct state* s)  { s->top--; *s->top += *(s->top + 1); DONEXT }
void Sub(struct state* s)  { s->top--; *s->top -= *(s->top + 1); DONEXT }
void Mul(struct state* s)  { s->top--; *s->top *= *(s->top + 1); DONEXT }
void Div(struct state* s)  { s->top--; *s->top /= *(s->top + 1); DONEXT }
void Mod(struct state* s)  { s->top--; *s->top %= *(s->top + 1); DONEXT }
void Neg(struct state* s)  { *s->top = -*s->top; DONEXT }
void Not(struct state* s)  { *s->top = !*s->top; DONEXT }
void Mod(struct state* s);
void Neg(struct state* s);
void Not(struct state* s);

void Swap(struct state* s) {
  intptr_t temp = *s->top;
  *s->top = *(s->top - 1);
  *(s->top - 1) = temp;
  DONEXT
}
void Over(struct state* s) { s->top++; *s->top = *(s->top - 2); DONEXT }
void Dup(struct state* s)  { s->top++; *s->top = *(s->top - 1); DONEXT }
void Decr(struct state* s) { (*s->top)--; DONEXT }
void Incr(struct state* s) { (*s->top)++; DONEXT }
void IsTrue(struct state* s) {
  s->pc++;
  if (*s->top--) s->pc += (intptr_t)(*s->pc);
  DONEXT
}
void IsFalse(struct state* s) {
  s->pc++;
  if (*s->top--) s->pc += (intptr_t)(*s->pc);
  DONEXT
}
void Jump(struct state* s) {
  s->pc++;
  s->pc += (intptr_t)(*s->pc);
  DONEXT
}

// From http://inglorion.net/software/deadbeef_rand/
static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;
void Random(struct state* s) {
  deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
  deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
  *s->top = deadbeef_seed % *s->top;
  DONEXT
}
void SeedRandom(struct state* s) {
  deadbeef_seed = *s->top--;
  deadbeef_beef = 0xdeadbeef;
  DONEXT
}

void Start(struct state* s, function* code) {
  s->top = s->stack - 1;
  s->pc = code;
  return (*s->pc)(s);
}
