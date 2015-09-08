#include "uscript.c"
#include <stdio.h>
#include <assert.h>

state_t S;

#ifndef ARDUINO
#ifdef __MACH__
  #include <mach/mach_time.h>
  unsigned long millis() {
    uint64_t time = mach_absolute_time();
    static uint64_t scaling_factor = 0;
    if (!scaling_factor) {
      mach_timebase_info_data_t info;
      mach_timebase_info(&info);
      scaling_factor = info.numer / info.denom;
    }
    return time * scaling_factor / 1000000;
  }
#else
  #include <time.h>
  unsigned long millis() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  }
#endif
#endif


int main() {
  printf("%td\n", sizeof(state_t));
  assert(sizeof(state_t) < 1500);
  while (loop(&S));
  return 0;
}
