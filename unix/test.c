#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "uscript.c"

void printString(const char* str) {
  printf("%s", str);
}
void printInt(int val) {
  printf("%d", val);
}
static int slots[16];
void slotWrite(int addr, int slot, int value) {
  printf("TODO: send %d to %d:%d\n", value, addr, slot);
}
int slotRead(int slot) {
  return slots[slot];
}
void pinMode(int pin, int mode) {
  printf("TODO: pinMode\n");
}
void digitalWrite(int pin, int value) {
  printf("TODO: digitalWrite\n");
}
int digitalRead(int pin) {
  printf("TODO: digitalRead\n");
  return 0;
}
void analogWrite(int pin, int value) {
  printf("TODO: analogWrite\n");
}
int analogRead(int pin) {
  printf("TODO: analogRead %d\n", pin);
  return 0;
}
void delayMicroseconds(int us) {

}
void delay(int ms) {
  return delayMicroseconds(ms * 1000);
}




#define Int16(val) ((uint16_t)(val) & 0xff), ((uint16_t)(val) >> 8)


void test(unsigned char* code, int count, int* expected) {
  top = stack - 1;
  int ret = eval(code);
  if (ret) {
    printf("Runtime error %d\n", ret);
    exit(ret);
  }
  int j;
  for (j = 0; j <= top - stack; j++) {
    printf("  %d: %d/%d\n", j, stack[j], expected[j]);
  }
  assert(top - stack + 1 == count);
  for (j = 0; j < count; j++) {
    assert(stack[j] == expected[j]);
  }
}

int main() {
  test((uint8_t[]){ END }, 0, (int[]){});
  test((uint8_t[]){ 1, 2, ADD, 3, 4, ADD, MUL, END }, 1, (int[]){ 21 });
  test((uint8_t[]){ 1, 2, AND, END }, 1, (int[]){ 2 });
  test((uint8_t[]){ 0, 2, AND, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 1, 0, AND, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 0, 0, AND, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 1, 2, OR, END }, 1, (int[]){ 1 });
  test((uint8_t[]){ 0, 2, OR, END }, 1, (int[]){ 2 });
  test((uint8_t[]){ 1, 0, OR, END }, 1, (int[]){ 1 });
  test((uint8_t[]){ 0, 0, OR, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 1, 2, XOR, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 0, 2, XOR, END }, 1, (int[]){ 2 });
  test((uint8_t[]){ 1, 0, XOR, END }, 1, (int[]){ 1 });
  test((uint8_t[]){ 0, 0, XOR, END }, 1, (int[]){ 0 });
  test((uint8_t[]){ 30, 21, BAND, END }, 1, (int[]){ 20 });
  test((uint8_t[]){ 30, 21, BOR, NEG, END }, 1, (int[]){ -31 });
  test((uint8_t[]){ 30, 21, BXOR, END }, 1, (int[]){ 11 });
  test((uint8_t[]){
    1, 2, ADD, 3, 4, ADD, MUL,
    CALL, 0x12, Int16(1),
    END,
    ISTC, Int16(5),
    42, 15,
    JMP, Int16(1),
    30, END
  }, 2, (int[]){ 21, 30 });
  return 0;
}
