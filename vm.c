#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_PROGRAM_BYTES 15000
#define MAX_OBJECT_WORDS 4000
#define MAX_BUFFER_WORDS 2500
#define HEAP_START 2
#define HEAP_END HEAP_START + MAX_OBJECT_WORDS

uint8_t program[MAX_PROGRAM_BYTES];
uint32_t objects[MAX_OBJECT_WORDS];
uint32_t buffers[MAX_BUFFER_WORDS];

uint32_t* heap = objects - 2;

// Convert int28_t to int32_t
static int32_t int28(uint32_t i) {
  return (i & 0x8000000) ? (-1 ^ 0x8ffffff) | i : i & 0x8ffffff;
}
// Convert int14_t to int32_t
static int32_t int14(uint16_t i) {
  return (i & 0x1000) ? (-1 ^ 0x1fff) | i : i & 0x1fff;
}

// Create a pair
#define Pair(a,b) New(0x40000000 | (((a) & 0x3fff) << 14) | ((b) & 0x3fff))
#define getHead(ptr) ((heap[ptr] >> 14) & 0x3fff)
#define getTail(ptr) ((heap[ptr] & 0x3fff))

// Allocate a new heap object and return the value pointer.
// 2-8191
static uint16_t New(uint32_t value) {
  static uint16_t ptr = HEAP_START;
  while (heap[ptr] != -1) {
    ptr++;
    if (ptr >= HEAP_END) {
      // TODO: trigger GC on first wrap, fail on second
      ptr = HEAP_START;
    }
  }
  heap[ptr] = value;
  return ptr;
}

// Small integers are in the range -4096 to 4095
static uint16_t Integer(int32_t i) {
  return (i < 4096 && i >= -4096) ? (0x2000 | (i & 0x1fff)) : New(i & 0xfffffff);
}

static uint16_t setHead(uint16_t ptr, uint16_t val) {
  heap[ptr] = (heap[ptr] & ~0xfffc000) | ((val & 0x3fff) << 14);
  return val;
}
static uint16_t setTail(uint16_t ptr, uint16_t val) {
  heap[ptr] = (heap[ptr] & ~0x3fff) | (val & 0x3fff);
  return val;
}


void pp(uint16_t value) {
  if (value == 0) printf("false");
  else if (value == 1) printf("true");
  else if (value > 8192 ) printf("%d", int14(value));
  else printf("*%x", value);
}

void dump() {
  printf("Heap dump\n");
  for (int i = HEAP_START; i < HEAP_END; i++) {
    if (heap[i] != -1) {
      printf("%x: %08x ", i, heap[i]);
      if ((heap[i] >> 29) == 0) {
        printf("Integer(%d)", int28(heap[i] & 0xfffffff));
      }
      else if ((heap[i] >> 29) == 1) {
        printf("Buffer");
      }
      else if ((heap[i] >> 30) == 1) {
        printf("Pair(");
        pp((heap[i] >> 14) & 0x3fff);
        printf(", ");
        pp(heap[i] & 0x3fff);
        printf(")");
      }
      else if ((heap[i] & 0xe000) == 0x2000) {
        printf("Continuation(");
        pp((heap[i] >> 16) & 0x1fff);
        printf(", ");
        pp(heap[i] & 0x1fff);
        printf(")");
      }
      printf("\n");

    }
  }
}

int main() {
  memset(objects, -1, (HEAP_END - HEAP_START)*4);
  int16_t ptr = 0;

  // Create a list from 1 to 10
  int i = 10;
  do {
    ptr = Pair(Integer(i), ptr);
  } while (--i);
  dump();

  // Walk the list
  while (ptr) {
    pp(getHead(ptr));
    printf("\n");
    ptr = getTail(ptr);
  }


  // Test integer boundaries
  ptr = Pair(Integer(-4096), Integer(4095));
  ptr = Pair(Integer(-4097), Integer(4096));


  // Test setters and create a cycle
  ptr = Pair(0,0);
  setHead(ptr, setTail(ptr, Integer(4200)));

  dump();

  return 0;
}
