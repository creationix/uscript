#include "uscript.c"
#include <stdio.h>
#include <stdlib.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[1;31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[1;34m"
#define KMAG  "\x1B[1;35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void myprint(int64_t num) {
  printf("%lld\r\n", num);
}

int main() {
  print_fn = &myprint;

  #ifdef BCM2708_PERI_BASE
  setup_io();
  #endif
  uint8_t* line = NULL;
  size_t size = 0;
  while (1) {
    process_events();

    printf(KNRM "> " KGRN);
    if (getline((char**)&line, &size, stdin) < 0) {
      printf(KNRM "\n");
      return 0;
    }
    printf(KNRM);
    int len = compile(line);
    if ((int) len < 0) {
      int offset = 1 - (int)len;
      while (offset--) printf(" ");
      printf(KRED "^\n");
      printf("Unexpected input\n" KNRM);
      continue;
    }
    uint8_t* program = line;
    while (program - line < len) {
      var result;
      program = eval(program, &result);
      printf("%s%lld%s\n", KBLU, result, KNRM);
    }
  }
}
