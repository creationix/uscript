#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_VALUE_STACK 256
typedef enum {
  // 0-127 start byte of multi-byte integer
  // 0mxxxxxx mxxxxxxx* Big endian
  DEF = 128, END, // @symbol DEF body... END
  CALL,           // @symbol CALL
  VAR,            // @symbol VAR
  STORE,          // value @symbol --
  FETCH,          // @symbol -- value
  IF, ELSE, THEN, // cond IF body... [ELSE body...] THEN
  DO, LOOP, I, J, // limit index DO body... LOOP
  BEGIN, UNTIL,   // BEGIN body... cond UNTIL
  WHILE, REPEAT,  // BEGIN body... cond WHILE body... REPEAT
  ADD, SUB, MUL, DIV, MOD,   // int int -- int
  NEG,                       // int -- int
  GT, LT, GTE, LTE, EQ, NEQ, // int int -- bool
  AND, OR, XOR,              // bool bool -- bool
  NOT,                       // bool -- bool
  DUP,                       // a -- a a
  DROP,                      // a --
  SWAP,                      // a b -- b a
  DEPTH,                     // -- n
  PEEK,                      // offset -- int32 - read memory at offset (big endian)
  PEEK8,                     // offset -- uint8 - read memory at offset
  PEEK16,                    // offset -- uint16 - read memory at offset (big endian)
  PEEK24,                    // offset -- uint24 - read memory at offset (big endian)
  PEEK32,                    // offset -- uint32 - read memory at offset (big endian)
  POKE,                      // int32 offset -- - write memory at offset (big endian)
  POKE8,                     // uint8 offset -- - write memory at offset
  POKE16,                    // uint16 offset -- - write memory at offset (big endian)
  POKE24,                    // uint24 offset -- - write memory at offset (big endian)
  POKE32,                    // uint32 offset -- - write memory at offset (big endian)
  FIN=191,
  // 192-255 start negative integer literal
  // 11mxxxxx mxxxxxxx* (more bit is inverted here)
} opcode_t;

typedef enum {
  SYMBOL,
  INTEGER,
  BOOLEAN,
} value_type_t;

typedef int32_t value_t;

typedef struct {
  value_t values[MAX_VALUE_STACK];
  value_type_t value_types[MAX_VALUE_STACK];
  int num_values;

} vm_t;

#include <stdio.h>

void dump(uint8_t* code) {
  while (*code != FIN) {
    printf("%02x: ", *code);
    if (*code < 0x80) {
      printf("MULTI\n");
      int32_t num = *code & 0x3f;
      if (*code++ & 0x40) {
        while (*code & 0x80) {
          num = (num << 7) | (*code++ & 0x7f);
        }
      }
      printf("INT %d\n", num);
      continue;
    }
    if (*code >= 0xc0) {
      int32_t num = (int8_t)(*code | 0x20);
      if (!(*code++ & 0x20)) {
        while (!(*code & 0x80)) {
          num = (num << 7) | (*code);
          code++;
        }
      }
      printf("NINT %d\n", num);
      continue;
    }
  }
}

int main() {

  dump((uint8_t[]){
    // Integers less than 64 or greater or equal to -64 represent themselves
    1, 2, 3, 50,
    // Larget integers take multiple bytes
    0x41,0, // should be
    -1, -2, -3,
    // 0xc0,156,
    FIN,
  });
  return 0;
}
