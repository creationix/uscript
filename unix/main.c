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

// From http://inglorion.net/software/deadbeef_rand/
static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;
static uint32_t deadbeef_rand() {
  deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
  deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
  return deadbeef_seed;
}
static void deadbeef_srand(uint32_t x) {
  deadbeef_seed = x;
  deadbeef_beef = 0xdeadbeef;
}

static unsigned char* Rand(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  *res = deadbeef_rand() % *res;
  return pc;
}

static unsigned char* Srand(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  deadbeef_srand(*res);
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

static unsigned char* List(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return pc;
  printf("TODO: list to stdout\n");
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
  {"RAND", Rand},       // (mod)
  {"SRAND", Srand},     // (seed)
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


  return 0;
}
