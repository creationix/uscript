#ifndef USCRIPT_H
#define USCRIPT_H

// VM tuning values
#define START_PAIRS 128
#define START_BYTES 1024
#define START_INTS 64

#include <stdint.h>
#include <stdbool.h>

/*

functional style cons list for lists
hash tree for maps

- hash key to 32-bit value with good pseudo random distribution
- walk down binary tree looking for key, each step is left or right depending on bit
  use each bit once
- if empty slot is found, store new value
- shape is ((key value) (left right)) where left and right recurse down the tree
- initially the map is just nil
- after the first stored value, it's ((key, value) nil)
- then depending on the bit needed to store the next value, either left or right
  becomes ((key value) nil) and so the top becomes ((key value) (left nil))

- set is the same, except you only store keys, not key/value pairs

- list is same, except there is no left/right split, just (value next)

Values are 31 bits wide, they use 4 bits for embedded type tag and 27 for value.

  0 true / false (0=false,1=true)
  1 int (int27_t)
  2 boxed int 32 or 64 (boxed values aren't allowed to be zero, so empty space is alreayd tagged)
  3 rational - pair of ints
  4 character (only 21 bits are needed for all of unicode 8.0's allocated space)
  5 string (utf-8 encoded unicode string)
  6 byte-array (8-bit)
  7 frame-buffer (32-bit)
  8 symbol (interned immutable ascii string) or anonymous
  9 pair
  10 stack
  11 set
  12 map

  13 native function
  14 closure function
  15 device

Pairs are 64-bits wide, this is 31 for each embedded value and 2 for GC flags

int32 and int64 tables use zero for empty space since it's not a valid value.

Buffer table has gc bit + 31-bit length and then data.  Non-compacting

All tables dynamically resize and trigger GC

TODO: figure out how static data in code works (dual addressing with heap data?)

*/

typedef enum type_e {
  BOOLEAN, INTEGER, BOX_INTEGER, RATIONAL,
  CHARACTER, STRING, SYMBOL, BYTE_ARRAY,
  FRAME_BUFFER, PAIR, STACK, SET,
  MAP, FUNCTION, CLOSURE, DEVICE
} type_t;

typedef struct value_s {
  bool gc : 1;
  type_t type : 4;
  int value : 27;
} value_t;

typedef struct pair_s {
  value_t left;
  value_t right;
} pair_t;

typedef struct state_s {
  pair_t* pairs; // Table of pairs
  uint8_t* bytes; // Buffer space for strings/byte-arrays/pixels
  int64_t* ints; // Table of int64 boxes
  int32_t num_pairs;
  int32_t next_pair;
  int32_t num_bytes;
  int32_t next_bytes;
  int32_t num_ints;
  int32_t _; // padding for alignment
} state_t;

// STATE
state_t* State();
void freeState(state_t* S);
uint8_t* eval(state_t* S, uint8_t* pc, value_t* out);

// NUMBERS
value_t Char(int32_t code);
value_t Bool(bool value);
value_t Int(int32_t value);
value_t Integer(state_t* S, int64_t value);
int64_t toInt(state_t* S, value_t value);
value_t Rational(state_t* S, int64_t n, int64_t d);

// BUFFERS
value_t String(state_t* S, int32_t length, const uint8_t* data);
value_t Symbol(state_t* S, int32_t length, const uint8_t* data);
value_t Buffer(state_t* S, int32_t length, const uint8_t* data);
value_t Pixels(state_t* S, int32_t length, const uint32_t* data);

// PAIRS
value_t Pair(state_t* S, value_t left, value_t right);
pair_t getPair(state_t* S, value_t slot);
value_t getLeft(state_t* S, value_t slot);
value_t getRight(state_t* S, value_t slot);
void setPair(state_t* S, value_t slot, value_t left, value_t right);
void setLeft(state_t* S, value_t slot, value_t value);
void setRight(state_t* S, value_t slot, value_t value);

// STACKS
value_t Stack(state_t* S);
value_t stackPush(state_t* S, value_t stack, value_t value);
value_t stackPeek(state_t* S, value_t stack);
value_t stackLength(state_t* S, value_t stack);
value_t stackPop(state_t* S, value_t stack);
value_t stackIs(state_t* S, value_t left, value_t right);
value_t stackReverse(state_t* S, value_t stack);

// SETS
value_t Set(state_t* S);
value_t setAdd(state_t* S, value_t set, value_t value);
value_t setHas(state_t* S, value_t set, value_t value);
value_t setRemove(state_t* S, value_t set, value_t value);

// MAPS
value_t Map(state_t* S);
value_t mapSet(state_t* S, value_t map, value_t key, value_t value);
value_t mapDelete(state_t* S, value_t map, value_t key);
value_t mapRead(state_t* S, value_t map, value_t key);

// BYTECODE

typedef enum opcode_e {
  LitFalse,
  LitTrue,
  LitInt, // num
  LitRational, // num num
  LitChar, // num
  LitString, // len:num byte*
  LitSymbol, // len:num byte*
  LitBuffer, // len:num byte*
  LitPixels, // len:num word*
  LitPair, // value value
  LitStack, // len:num value*
  LitSet, // len:num value*
  LitMap, // len:num (value/value)*
  Block, // len:num value*
} opcode_t;


// integer is encoded msxxxxx mxxxxxxx* where s is sign (1 for negative) and
// m is more.  All but the last byte will have m set 1.
// Holds ints from -64 to 64
#define Int7(x) \
          0x7f & x
// Holds ints from -8192 to 8191
#define Int14(x) \
  0x80 | (0x7f & (x >> 7)),\
          0x7f & x
// Holds ints from -1048576 to 1048575
#define Int21(x) \
  0x80 | (0x7f & (x >> 14)),\
  0x80 | (0x7f & (x >> 7)),\
          0x7f & x
// Holds ints from -134217728 to 134217727
#define Int28(x) \
0x80 | (0x7f & (x >> 21)),\
0x80 | (0x7f & (x >> 14)),\
0x80 | (0x7f & (x >> 7)),\
        0x7f & x
// Holds ints from -17179869184 to 17179869183
#define Int35(x) \
0x80 | (0x7f & (x >> 28)),\
0x80 | (0x7f & (x >> 21)),\
0x80 | (0x7f & (x >> 14)),\
0x80 | (0x7f & (x >> 7)),\
        0x7f & x
#define Uint32(x) \
0xff & (x >> 24),\
0xff & (x >> 16),\
0xff & (x >> 8),\
0xff & x

#endif
