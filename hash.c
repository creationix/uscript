#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>

#define START_PAIRS 10
#define START_BYTES 100
#define START_INTS 3

typedef enum type_e {
  BOOLEAN = 0,
  INTEGER = 1,
  BOX_INTEGER = 2,
  RATIONAL = 3,
  CHARACTER = 4,
  STRING = 5,
  SYMBOL = 6,
  BYTE_ARRAY = 7,
  FRAME_BUFFER = 8,
  PAIR = 9,
  STACK = 10,
  SET = 11,
  MAP = 12,
  FUNCTION = 13,
  CLOSURE = 14,
  DEVICE = 15,
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
  int32_t num_ints;
} state_t;

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




static void prettyPrint(state_t* S, value_t value);


static state_t* State() {
  state_t* S = malloc(sizeof(state_t));
  S->num_pairs = START_PAIRS;
  S->next_pair = 0;
  S->pairs = calloc(START_PAIRS, sizeof(pair_t));
  S->num_bytes = START_BYTES;
  S->bytes = calloc(START_BYTES, sizeof(uint8_t));
  S->num_ints = START_INTS;
  S->ints = calloc(START_INTS, sizeof(int64_t));
  return S;
}

static bool eq(value_t left, value_t right) {
  return left.type == right.type && left.value == right.value;
}

static bool falsy(value_t value) {
  return value.type == BOOLEAN && !value.value;
}

static bool truthy(value_t value) {
  return !falsy(value);
}

static value_t Char(int32_t code) {
  return (value_t){
    .gc = true,
    .type = CHARACTER,
    .value = code
  };
}

static value_t Bool(bool value) {
  return (value_t){
    .gc = true,
    .type = BOOLEAN,
    .value = value
  };
}

static value_t Int(int32_t value) {
  return (value_t){
    .gc = true,
    .type = INTEGER,
    .value = value
  };
}

static value_t Integer(state_t* S, int64_t value) {
  // If the value fits within the int27 range, store unboxed.
  if (value >= -0x4000000 && value < 0x4000000) {
    return Int((int32_t)value);
  }

  // Otherwise it must be stored in an int64 field.
  int32_t i = 0;
  while (true) {
    int64_t slot = S->ints[i];
    if (!slot) {
      S->ints[i] = value;
      break;
    }
    if (slot == value) break;
    i++;
    if (i < S->num_ints) {
      continue;
    }
    int32_t j = i + START_INTS;
    S->num_ints = j;
    S->ints = realloc(S->ints,  sizeof(int64_t) * (size_t)j);
    while (j-- > i) {
      S->ints[j] = 0;
    }
  }
  return (value_t){
    .gc = true,
    .type = BOX_INTEGER,
    .value = i
  };
}

static int64_t toInt(state_t* S, value_t value) {
  switch (value.type) {
    case INTEGER: return value.value;
    case BOX_INTEGER: return S->ints[value.value];
    default: return 0; // TODO: should this thow an error?
  }
}

static pair_t cons(value_t left, value_t right) {
  return (pair_t){
    .left = left,
    .right = right
  };
}

static value_t Pair(state_t* S, type_t type, value_t left, value_t right) {
  // Find an empty slot in the pairs table
  int32_t i = S->next_pair;
  while (true) {
    if (i >= S->num_pairs) {
      // TODO: gc
      int32_t old_length = S->num_pairs;
      int32_t new_length = old_length + START_PAIRS;
      size_t old_size = (size_t)old_length * sizeof(pair_t);
      size_t new_size = (size_t)new_length * sizeof(pair_t);
      S->pairs = realloc(S->pairs, new_size);
      memset((uint8_t*)S->pairs + old_size, 0, new_size - old_size);
      S->num_pairs = new_length;
      continue;
    }
    pair_t pair = S->pairs[i];
    if (!(pair.left.gc || pair.right.gc)) {
      // If both GC flags are zero, assume it's free.
      break;
    }
    ++i;
  }
  S->next_pair = i + 1;
  S->pairs[i] = cons(left, right);
  return (value_t){
    .gc = true,
    .type = type,
    .value = i
  };
}

static pair_t getPair(state_t* S, value_t value) {
  return S->pairs[value.value];
}

static void setPair(state_t* S, value_t value, pair_t pair) {
  S->pairs[value.value] = pair;
}

static value_t Stack(state_t* S) {
  return Pair(S, STACK, Int(0), Bool(false));
}

// Push a value onto a stack value
static void stackPush(state_t* S, value_t stack, value_t value) {
  assert(stack.type == STACK);
  pair_t meta = getPair(S, stack);
  int64_t count = toInt(S, meta.left);
  setPair(S, stack, cons(
    Integer(S, count + 1),
    Pair(S, PAIR, value, meta.right)
  ));
}

// Pop a value from a stack value (or return false if empty)
static value_t stackPop(state_t* S, value_t stack) {
  assert(stack.type == STACK);
  pair_t meta = getPair(S, stack);
  int64_t count = toInt(S, meta.left);
  if (!count) { return Bool(false); }
  pair_t first = getPair(S, meta.right);
  setPair(S, stack, cons(
    Integer(S, count - 1),
    first.right
  ));
  return first.left;
}

static int32_t hash(value_t value) {
  int32_t num = value.type << 31 | value.value;
  return (num >> 13) ^ num ^ (num << 12);
}

static value_t Set(state_t* S) {
  return Pair(S, SET, Bool(false), Bool(false));
}

static bool recursiveAdd(state_t* S, value_t set, value_t value, int32_t bits) {
  pair_t node = getPair(S, set);
  value_t payload = node.left;
  if (eq(payload, value)) return false;
  value_t tree = node.right;
  // If the slot is empty, insert the value there.
  if (falsy(payload)) {
    setPair(S, set, cons(value, tree));
    return true;
  }
  // If there is no tree/split yet, create one with the value already in it.
  if (falsy(tree)) {
    value_t next = Pair(S, PAIR, value, Bool(false));
    setPair(S, set, cons(
      payload,
      bits & 1 ?
        Pair(S, PAIR, next, Bool(false)) :
        Pair(S, PAIR, Bool(false), next)
    ));
    return true;
  }
  // Otherwise, check the side for space.
  pair_t split = getPair(S, tree);

  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  if (truthy(side)) return recursiveAdd(S, side, value, bits >> 1);

  // If not, fill it out
  if (bits & 1) {
    setPair(S, tree, cons(
      Pair(S, PAIR, value, Bool(false)),
      split.right
    ));
    return true;
  }
  else {
    setPair(S, tree, cons(
      split.left,
      Pair(S, PAIR, value, Bool(false))
    ));
    return true;
  }
}

static value_t setAdd(state_t* S, value_t set, value_t value) {
  assert(set.type == SET);
  return Bool(recursiveAdd(S, set, value, hash(value)));
}

static value_t Map(state_t* S) {
  return Pair(S, MAP, Bool(false), Bool(false));
}

static void prettyPrint(state_t* S, value_t value);

static void printSetNode(state_t* S, value_t node, bool later) {
  if (falsy(node)) return;
  pair_t pair = getPair(S, node);
  node = pair.left;
  if (truthy(node)) {
    if (later) putchar(' ');
    prettyPrint(S, node);
    later = true;
  }
  node = pair.right;
  if (falsy(node)) return;
  pair_t split = getPair(S, node);
  printSetNode(S, split.right, later);
  printSetNode(S, split.left, later);
}

// static void printMapNode(state_t* S, value_t node, bool later) {
//   if (!node) return;
//   pair_t pair = getPair(S, node);
//   value_t value = left(pair);
//   if (value) {
//     if (later) putchar(' ');
//     pair_t mapping = getPair(S, value);
//     prettyPrint(S, left(mapping));
//     printf(":");
//     prettyPrint(S, right(mapping));
//     later = true;
//   }
//   value = right(pair);
//   if (!value) return;
//   pair_t split = getPair(S, value);
//   printMapNode(S, right(split), later);
//   printMapNode(S, left(split), later);
// }

static void prettyPrint(state_t* S, value_t value) {
  switch (value.type) {
    case BOOLEAN:
      printf("%s", value.value ? "true" : "false");
      break;
    case INTEGER:
    case BOX_INTEGER:
      printf("%"PRId64, toInt(S, value));
      break;
    case RATIONAL: {
      pair_t pair = getPair(S, value);
      prettyPrint(S, pair.left);
      putchar('/');
      prettyPrint(S, pair.right);
      break;
    }
    case CHARACTER:
      putchar('\'');
      if (value.value < 0x20) {
        switch (value.value) {
          case 7: printf("\\a"); break;
          case 8: printf("\\b"); break;
          case 9: printf("\\t"); break;
          case 10: printf("\\n"); break;
          case 11: printf("\\v"); break;
          case 12: printf("\\f"); break;
          case 13: printf("\\r"); break;
          default: printf("\\%d", value.value); break;
        }
      }
      // Encode the value as UTF-8 bytes
      else if (value.value < 0x80) {
        putchar((signed)value.value);
      }
      else if (value.value < 0x800) {
        putchar(0xc0 | (value.value >> 6));
        putchar(0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x10000) {
        putchar(0xe0 | (value.value >> 12));
        putchar(0x80 | ((value.value >> 6) & 0x3f));
        putchar(0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x200000) {
        putchar(0xf0 | (value.value >> 18));
        putchar(0x80 | ((value.value >> 12) & 0x3f));
        putchar(0x80 | ((value.value >> 6) & 0x3f));
        putchar(0x80 | (value.value & 0x3f));
      }
      else {
        printf("\\u%d", value.value);
      }
      putchar('\'');
      break;
    case STRING:
      printf("TODO: print STRING");
      break;
    case SYMBOL:
      printf("TODO: print SYMBOL");
      break;
    case BYTE_ARRAY:
      printf("TODO: print BYTE_ARRAY");
      break;
    case FRAME_BUFFER:
      printf("TODO: print FRAME_BUFFER");
      break;
    case PAIR: case MAP: {
      pair_t pair = getPair(S, value);
      putchar('(');
      prettyPrint(S, pair.left);
      putchar(' ');
      prettyPrint(S, pair.right);
      putchar(')');
      break;
    }
    case STACK: {
      putchar('[');
        pair_t pair = getPair(S, value);
        value_t next;
        bool later = false;
        while (truthy(next = pair.right)) {
          pair = getPair(S, next);
          if (later) putchar(' ');
          later = true;
          prettyPrint(S, pair.left);
        }
      }
      putchar(']');
      break;
    case SET:
      putchar('<');
      printSetNode(S, value, false);
      putchar('>');
      break;
    // case MAP:
    //   putchar('{');
    //   printMapNode(S, value, false);
    //   putchar('}');
    //   break;
    case FUNCTION:
      printf("TODO: print FUNCTION");
      break;
    case CLOSURE:
      printf("TODO: print CLOSURE");
      break;
    case DEVICE:
      printf("TODO: print DEVICE");
      break;
  }
}

static void dump(state_t* S, value_t value) {
  prettyPrint(S, value);
  putchar('\n');
}

int main() {
  assert(sizeof(value_t) == 4);
  assert(sizeof(pair_t) == 8);
  state_t* S = State();
  // for (int64_t i = 1; i > 0 && i <= INT64_MAX; i <<= 4) {
  //   printf("Testing %"PRId64"\n", i);
  //   dump(S, Integer(S, i));
  //   dump(S, Integer(S, -i));
  // }
  // dump(S, Integer(S, 0));
  // dump(S, Integer(S, 1));
  // dump(S, Integer(S, -1));
  // dump(S, Bool(true));
  // dump(S, Bool(false));
  // dump(S, Pair(S, PAIR,
  //   Integer(S, 1),
  //   Integer(S, 2)));
  // dump(S, Pair(S, RATIONAL,
  //   Integer(S, 1),
  //   Integer(S, 2)));
  // for (int i = 0; i < 100; i++) {
  //   dump(S, Char(i));
  // }
  // dump(S, Char('@'));
  // dump(S, Char(9654));   // ▶
  // dump(S, Char(128513)); // 😁
  // dump(S, Char(128525)); // 😍

  value_t stack = Stack(S);
  dump(S, stack);
  stackPush(S, stack, Char('A'));
  dump(S, stack);
  // stackPush(S, stack, Char('B'));
  // dump(S, stack);
  // stackPush(S, stack, Char('C'));
  // dump(S, stack);
  // dump(S, stackPop(S, stack));
  // dump(S, stack);
  // dump(S, stackPop(S, stack));
  // dump(S, stack);
  // dump(S, stackPop(S, stack));
  // dump(S, stack);
  dump(S, stackPop(S, stack));
  dump(S, stack);

  value_t set = Set(S);
  dump(S, set);
  dump(S, setAdd(S, set, Char('J')));
  dump(S, set);
  dump(S, setAdd(S, set, Char('J')));
  dump(S, set);
  dump(S, setAdd(S, set, Int(20)));
  dump(S, set);
  dump(S, setAdd(S, set, Int(20)));
  dump(S, set);
  dump(S, setAdd(S, set, Int(30)));
  dump(S, set);
  dump(S, setAdd(S, set, Int(40)));
  dump(S, set);
  dump(S, setAdd(S, set, Int(50)));
  dump(S, set);
  dump(S, setAdd(S, set, Int(60)));
  dump(S, set);

  value_t map = Map(S);
  dump(S, map);
}
