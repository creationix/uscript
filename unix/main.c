// #!/usr/bin/tcc -run -Wall -Werror

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>


// Assume unix on arm is raspberry pi for now
// We need much better logic in the long run.
#ifdef __arm__
#include "rpi-io.c"
#endif

static volatile int stopLoop = 0;

#define NUMBER_TYPE int64_t
#define DATA_FILE "uscript.dat"
#define CHECKER stopLoop
#include "uscript.c"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void intHandler(int dummy) {
  stopLoop = 1;
}

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
  int i;
  int o = 2;
  FILE* fd = fopen(DATA_FILE, "w");
  fputc('u', fd);
  for (i = 0; i < SIZE_STUBS; i++) {
    if (!S->stubs[i]) continue;
    printf("Saving %c...\n", i + 'a');
    int len = skip(S, S->stubs[i]) - S->stubs[i];
    o += len + 3;
    fputc(i, fd);
    fputc(len >> 8, fd);
    fputc(len & 0xff, fd);
    int j;
    for (j = 0; j < len; j++) {
      fputc(S->stubs[i][j], fd);
    }
  }
  fputc('u', fd);
  fclose(fd);
  *res = o;
  return pc;
}

#ifdef BCM2708_PERI_BASE

static unsigned char* PinMode(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin;
  pc = eval(S, pc, &pin);
  pc = eval(S, pc, res);
  INP_GPIO(pin);
  if (*res) OUT_GPIO(pin);
  return pc;
}

static unsigned char* DigitalWrite(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin;
  pc = eval(S, pc, &pin);
  pc = eval(S, pc, res);
  if (*res) GPIO_SET = 1 << pin;
  else GPIO_CLR = 1 << pin;
  return pc;
}

static unsigned char* AnalogWrite(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin;
  pc = eval(S, pc, &pin);
  pc = eval(S, pc, res);
  // TODO: pwm write somehow?
  printf("TODO: Implement analog write for rPI\n");
  return pc;
}

static unsigned char* DigitalRead(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  *res = !!GET_GPIO(pin);
  return pc;
}

static unsigned char* AnalogRead(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  // TODO: there are no analog inputs right?
  printf("TODO: Implement analog read for rPI\n");
  *res = 0;
  return pc;
}

static unsigned char* Tone(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, skip(S, pc)));
  number pin, dur;
  pc = eval(S, pc, &pin);
  pc = eval(S, pc, res);
  pc = eval(S, pc, &dur);
  struct timeval t;
  t.tv_sec = 0;

  int p = 1000000 / *res;
  dur *= 1000;
  t.tv_usec = p >> 1;
  while ((dur -= p) > 0) {
    GPIO_SET = 1 << pin;
    select(0, NULL, NULL, NULL, &t);
    GPIO_CLR = 1 << pin;
    select(0, NULL, NULL, NULL, &t);
  }
  return pc;
}

#endif

static struct uState S;

static struct user_func funcs[] = {
  #ifdef BCM2708_PERI_BASE
  {"PM", PinMode},      // (pin, mode) -> mode
  {"DW", DigitalWrite}, // (pin, value) -> value
  {"DR", DigitalRead},  // (pin) -> value
  {"AW", AnalogWrite}, // (pin, value) -> value
  {"AR", AnalogRead},  // (pin) -> value
  {"TONE", Tone},       // (pin, frequency, duration) -> frequency
  #endif
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

  #ifdef BCM2708_PERI_BASE
  setup_io();
  #endif

  printf("Welcome to uscript.\n");
  FILE* fd = fopen(DATA_FILE, "r");
  if (fd) {
    if (fgetc(fd) == 'u') {
      while (1) {
        int key = fgetc(fd);
        if (key == 'u') break;
        printf("Loading %c...\n", key + 'a');
        int len = fgetc(fd);
        len = (len << 8) | fgetc(fd);
        uint8_t* stub = S.stubs[key] = malloc(len);
        int j;
        for (j = 0; j < len; j++) {
          stub[j] = fgetc(fd);
        }
      }

    }
    fclose(fd);
  }
  if (S.stubs[0]) {
    printf("Running auto script...\n");
    number out;
    signal(SIGINT, intHandler);
    stopLoop = 0;
    eval(&S, S.stubs[0], &out);
    signal(SIGINT, SIG_DFL);
  }

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
      signal(SIGINT, intHandler);
      stopLoop = 0;
      program = eval(&S, program, &result);
      signal(SIGINT, SIG_DFL);
      printf("%s%"PRId64"%s\n", KBLU, result, KNRM);
    }
  }

  return 0;
}
