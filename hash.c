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

static value_t RawPair(state_t* S, type_t type, pair_t pair) {
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
    pair_t slot = S->pairs[i];
    if (!(slot.left.gc || slot.right.gc)) {
      // If both GC flags are zero, assume it's free.
      break;
    }
    ++i;
  }
  S->next_pair = i + 1;
  S->pairs[i] = pair;
  return (value_t){
    .gc = true,
    .type = type,
    .value = i
  };
}

static value_t Pair(state_t* S, pair_t pair) {
  return RawPair(S, PAIR, pair);
}

static value_t Rational(state_t* S, int64_t n, int64_t d) {
  return RawPair(S, RATIONAL, cons(Integer(S, n), Integer(S, d)));
}

static pair_t getPair(state_t* S, value_t value) {
  return S->pairs[value.value];
}

static void setPair(state_t* S, value_t slot, pair_t pair) {
  S->pairs[slot.value] = pair;
}

static pair_t updateLeft(pair_t pair, value_t value) {
  return (pair_t){
    .left = value,
    .right = pair.right
  };
}

static pair_t updateRight(pair_t pair, value_t value) {
  return (pair_t){
    .left = pair.left,
    .right = value
  };
}

static pair_t updateSide(pair_t pair, int side, value_t value) {
  return side ? updateLeft(pair, value) : updateRight(pair, value);
}

static void setLeft(state_t* S, value_t slot, value_t value) {
  S->pairs[slot.value] = updateLeft(S->pairs[slot.value], value);
}

// static void setRight(state_t* S, value_t slot, value_t value) {
//   S->pairs[slot.value] = updateRight(S->pairs[slot.value], value);
// }
//

static value_t Stack(state_t* S) {
  return RawPair(S, STACK, cons(Int(0), Bool(false)));
}

static value_t stackPush(state_t* S, value_t stack, value_t value) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  value_t count = Integer(S, toInt(S, meta.left) + 1);
  setPair(S, stack, cons(
    count,
    Pair(S, cons(value, meta.right))
  ));
  return count;
}

static value_t stackPeek(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  if (meta.right.type == PAIR) {
    pair_t next = getPair(S, meta.right);
    return next.left;
  }
  return meta.right;
}

static value_t stackLength(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  return meta.left;
}

static value_t stackPop(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
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


// Used by set and map. A 32-bit hash is generated for every value so that it
// takes a pseudo random path down the tree for fast search.
static int32_t hash(value_t value) {
  return (value.type << 27 | value.value) ^ value.type ^ value.value << value.type;
}

static value_t Set(state_t* S) {
  return RawPair(S, SET, cons(Bool(false), Bool(false)));
}

// setNode = (value, split)
// setNode = (value, (false, rightNode))
// setNode = (value, (leftNode, false))
// setNode = (value, (leftNode, rightNode))
// An false value means the slot is empty (it might have held a deleted value)
// An false split means both sides are unset
// The split may have one side false and the other set.
//
// recursiveAdd needs to search for a matching value.
// If the value is not found, then the first empty slot that was encountered
// is used to add the value.  If there was none, add a new slot (and possibly
// a new split to hold it)
static bool recursiveAdd(state_t* S, value_t set, value_t value, value_t slot, int32_t bits) {
  pair_t node = getPair(S, set);
  // If the value is already here, abort.
  if (eq(node.left, value)) return false;

  // If we find a free slot and havn't seen one yet, record it.
  if (falsy(node.left) && slot.type == BOOLEAN) {
    slot = set;
  }

  // If there are no further subtrees, stop here.
  if (node.right.type != PAIR) {
    // If we had seen an empty slot, use it.
    if (slot.type != BOOLEAN) {
      setLeft(S, slot, value);
      return true;
    }
    // Otherwise, create a new split and populate one side with slot and value.
    slot = Pair(S, cons(value, Bool(false)));
    setPair(S, set, cons(node.left, Pair(S, bits & 1
      ? cons(slot, Bool(false))
      : cons(Bool(false), slot))));
    return true;
  }

  // If there is a subtree on the matching side, recurse into it.
  pair_t split = getPair(S, node.right);
  value_t side = bits & 1 ? split.left : split.right;
  if (side.type == PAIR) {
    return recursiveAdd(S, side, value, slot, bits >> 1);
  }

  // If we've reached this end and had seen a slot, use it now.
  if (slot.type != BOOLEAN) {
    setLeft(S, slot, value);
    return true;
  }

  // Otherwise fill out the other half of the split.
  side = Pair(S, cons(value, Bool(false)));
  setPair(S, node.right, updateSide(split, bits & 1, side));
  return true;
}

static value_t setAdd(state_t* S, value_t set, value_t value) {
  assert(set.type == SET);
  return Bool(recursiveAdd(S, set, value, Bool(false), hash(value)));
}

static bool recursiveHas(state_t* S, value_t set, value_t value, int32_t bits) {
  pair_t node = getPair(S, set);
  // If we find the value, we're done!
  if (eq(node.left, value)) return true;
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveHas(S, side, value, bits >> 1);
}

static value_t setHas(state_t* S, value_t set, value_t value) {
  assert(set.type == SET);
  return Bool(recursiveHas(S, set, value, hash(value)));
}

static bool recursiveDel(state_t* S, value_t set, value_t value, int32_t bits) {
  pair_t node = getPair(S, set);
  // If we find the value, remove it and we're done!
  if (eq(node.left, value)) {
    setPair(S, set, cons(
      Bool(false),
      node.right));
    return true;
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveDel(S, side, value, bits >> 1);
}

static value_t setDel(state_t* S, value_t set, value_t value) {
  assert(set.type == SET);
  return Bool(recursiveDel(S, set, value, hash(value)));
}

/*
static value_t Map(state_t* S) {
  return Pair(S, MAP, Bool(false), Bool(false));
}

static bool recursiveSet(state_t* S, value_t map, value_t key, value_t value, int32_t bits) {
  pair_t node = getPair(S, map);

  // If the key matches, set the value.
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) {
      // Only return true if the value changed
      if (!eq(mapping.right, value)) {
        setPair(S, node.left, cons(key, value));
        return true;
      }
      return false;
    }
  }

  // If there is no split, create a new one with mapping in it.
  if (node.right.type != PAIR) {
    setPair(S, map, cons(
      node.left,

    )())
  }

  pair_t split = getPair(S, node.right);
  value_t side = (bits & 1) ? split.left : split.right;
  // If the path exists, recurse down it.
  if (side.type == PAIR) {
    return recursiveSet(S, side, key, value, bits >> 1);
  }
  // Fill out the empty side with new entry
  setPair(S, node.right, updateSide(split, bits & 1,
    Pair(S, PAIR,
      Pair(S, PAIR, key, value), Bool(false))));
  return true;
}

static value_t mapSet(state_t* S, value_t map, value_t key, value_t value) {
  assert(map.type == MAP);
  return Bool(recursiveSet(S, map, key, value, hash(key)));
}
*/

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

int main() {
  deadbeef_srand(42);
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

  printf("\n--STACKS--\n");
  printf("Creating an empty stack\n");
  value_t stack = Stack(S);
  dump(S, stack);
  printf("Pushing 'A' onto the stack\n");
  dump(S, stackPush(S, stack, Char('A')));
  dump(S, stack);
  printf("Pushing 'B' onto the stack\n");
  dump(S, stackPush(S, stack, Char('B')));
  dump(S, stack);
  printf("Pushing 'C' onto the stack\n");
  dump(S, stackPush(S, stack, Char('C')));
  dump(S, stack);
  printf("Peeking on the stack\n");
  dump(S, stackPeek(S, stack));
  printf("Reading stack length\n");
  dump(S, stackLength(S, stack));
  printf("Popping from the stack\n");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the stack\n");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the stack\n");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the empty stack\n");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Peeking on the empty stack\n");
  dump(S, stackPeek(S, stack));
  printf("Reading empty stack length\n");
  dump(S, stackLength(S, stack));

  printf("\n--SETS--\n");
  printf("Creating ane empty set\n");
  value_t set = Set(S);
  dump(S, set);
  for (int i = 0; i < 10; i++) {
    int n = deadbeef_rand() % 20;
    printf("set.add(%d) -> ", n);
    dump(S, setAdd(S, set, Int(n)));
    dump(S, set);
    n = deadbeef_rand() % 20;
    printf("set.del(%d) -> ", n);
    dump(S, setDel(S, set, Int(n)));
    dump(S, set);
    for (int j = 0; j < 20; j++) {
      putchar(truthy(setHas(S, set, Int(j))) ? '#' : '_');
    }
    putchar('\n');
  }
  dump(S, setAdd(S, set, Pair(S, cons(Bool(true),Char('b')))));
  dump(S, set);
  dump(S, setAdd(S, set, Rational(S, 1, 3)));
  dump(S, set);
  // value_t map = Map(S);
  // dump(S, map);
  // dump(S, mapSet(S, map, Int(10), Int(20)));
  // dump(S, map);
}
