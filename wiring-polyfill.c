#include <stdio.h>
#include <sys/select.h>

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

void delay(int ms) {
  struct timeval tv;
  tv.tv_sec = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
  select(0, 0, 0, 0, &tv);
}

void pinMode(uint8_t pin, uint8_t mode) {
  printf("pinMode(%d, %d)\n", pin, mode);
}
void digitalWrite(uint8_t pin, uint8_t value) {
  printf("digitalWrite(%d, %d)\n", pin, value);
}
int digitalRead(uint8_t pin) {
  printf("digitalRead(%d)\n", pin);
  return 0;
}
int analogRead(uint8_t pin) {
  printf("analogRead(%d)\n", pin);
  return 0;
}
void analogWrite(uint8_t pin, int value) {
  printf("analogWrite(%d, %d)\n", pin, value);
}
