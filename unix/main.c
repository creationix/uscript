// #!/usr/bin/tcc -run -Wall -Werror

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/time.h>

#define NUMBER_TYPE int64_t
#include "uscript.c"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

static unsigned char* Print(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  printf("%"PRId64"\n", *res);
  return pc;
}

static unsigned char* Delay(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  struct timeval t;
  t.tv_sec = *res / 1000;
  t.tv_usec = (*res % 1000) * 1000;
  select(0, NULL, NULL, NULL, &t);
  return pc;
}

void dump(struct uState* S, uint8_t* pc, int len) {
  uint8_t* end = pc + len;
  while (pc < end) {
    // If the high bit is set, it's an opcode index.
    if (*pc & 0x80) {
      printf(" %s", op_to_name(S, *pc++));
      continue;
    }
    // Otherwise it's a variable length encoded integer.
    number val = *pc & 0x3f;
    if (*pc++ & 0x40) {
      int b = 6;
      do {
        val |= (number)(*pc & 0x7f) << b;
        b += 7;
      } while (*pc++ & 0x80);
    }
    printf(" %"PRId64, val);
  }
}

static unsigned char* List(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return pc;
  int i;
  *res = 2;
  for (i = 0; i < SIZE_STUBS; i++) {
    if (!S->stubs[i]) continue;
    printf("DEF %c", i + 'a');
    int len = skip(S, S->stubs[i]) - S->stubs[i];
    dump(S, S->stubs[i], len);
    printf("\n");
    *res += len + 3;
  }
  return pc;
}

static unsigned char* Save(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return pc;
  printf("TODO: save to filesystem\n");
  return pc;
}

static struct uState S;

static struct user_func funcs[] = {
  {"DELAY", Delay},     // (ms)
  {"PRINT", Print},     // (num)
  {"LIST", List},       //
  {"SAVE", Save},       //
  {NULL}
};

int main() {

  S.malloc = malloc;
  S.free = free;
  S.funcs = funcs;
  S.num_funcs = 0;
  while (funcs[S.num_funcs].name) S.num_funcs++;

  uint8_t* line = NULL;
  size_t size = 0;
  while (1) {

    printf(KNRM "> " KGRN);
    if (getline((char**)&line, &size, stdin) < 0) {
      printf(KNRM "\n");
      return 0;
    }
    printf(KNRM);
    int len = compile(&S, line);
    if ((int) len < 0) {
      int offset = 1 - (int)len;
      while (offset--) printf(" ");
      printf(KRED "^\n");
      printf("Unexpected input\n" KNRM);
      continue;
    }
    uint8_t* program = line;
    while (program - line < len) {
      number result;
      program = eval(&S, program, &result);
      printf("%s%"PRId64"%s\n", KBLU, result, KNRM);
    }
  }

  return 0;
}
