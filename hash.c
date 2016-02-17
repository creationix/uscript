#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>

#define START_PAIRS 10
#define START_BYTES 1024
#define START_INTS 3

#define RED 0xff2200
#define ORN 0xf08000
#define YEL 0xe8e800
#define GRN 0x22ee00
#define BLU 0x0088ff
#define PUR 0x9000f0
#define OFF 0x000000

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

typedef struct buffer_s {
  bool gc : 1;
  int length: 31;
  uint8_t data[];
} buffer_t;

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

static value_t stackIs(state_t* S, value_t left, value_t right) {
  if (left.type != STACK || right.type != STACK) return Bool(false);
  pair_t lpair = getPair(S, left);
  pair_t rpair = getPair(S, right);
  // Make sure lengths match up.
  if (!eq(lpair.left, rpair.left)) return Bool(false);

  while (lpair.right.type == PAIR) {
    lpair = getPair(S, lpair.right);
    rpair = getPair(S, rpair.right);
    if (!eq(lpair.left, rpair.left)) return Bool(false);
  }
  return Bool(true);
}

static value_t stackReverse(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  value_t node = meta.right;
  value_t last = Bool(false);
  while (node.type == PAIR) {
    pair_t link = getPair(S, node);
    last = Pair(S, cons(link.left, last));
    node = link.right;
  }
  return RawPair(S, STACK, cons(meta.left, last));
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

static bool recursiveRemove(state_t* S, value_t set, value_t value, int32_t bits) {
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
  return side.type == PAIR && recursiveRemove(S, side, value, bits >> 1);
}

static value_t setRemove(state_t* S, value_t set, value_t value) {
  assert(set.type == SET);
  return Bool(recursiveRemove(S, set, value, hash(value)));
}


static value_t Map(state_t* S) {
  return RawPair(S, MAP, cons(Bool(false), Bool(false)));
}

static bool recursiveSet(state_t* S, value_t map, value_t key, value_t value, value_t slot, int32_t bits) {
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

  // If we find a free slot and havn't seen one yet, record it.
  if (falsy(node.left) && slot.type == BOOLEAN) {
    slot = map;
  }

  // If there are no further subtrees, stop here.
  if (node.right.type != PAIR) {
    // If we had seen an empty slot, use it.
    if (slot.type != BOOLEAN) {
      setLeft(S, slot, Pair(S, cons(key, value)));
      return true;
    }
    // Otherwise, create a new split and populate one side with slot and value.
    slot = Pair(S, cons(Pair(S, cons(key, value)), Bool(false)));
    setPair(S, map, cons(node.left, Pair(S, bits & 1
      ? cons(slot, Bool(false))
      : cons(Bool(false), slot))));
    return true;
  }

  // If there is a subtree on the matching side, recurse into it.
  pair_t split = getPair(S, node.right);
  value_t side = bits & 1 ? split.left : split.right;
  if (side.type == PAIR) {
    return recursiveSet(S, side, key, value, slot, bits >> 1);
  }

  // If we've reached this end and had seen a slot, use it now.
  if (slot.type != BOOLEAN) {
    setLeft(S, slot, Pair(S, cons(key, value)));
    return true;
  }

  // Otherwise fill out the other half of the split.
  side = Pair(S, cons(Pair(S, cons(key, value)), Bool(false)));
  setPair(S, node.right, updateSide(split, bits & 1, side));
  return true;
}

static value_t mapSet(state_t* S, value_t map, value_t key, value_t value) {
  if (map.type != MAP) return Bool(false);
  return Bool(recursiveSet(S, map, key, value, Bool(false), hash(key)));
}

static bool recursiveDelete(state_t* S, value_t map, value_t key, int32_t bits) {
  pair_t node = getPair(S, map);
  // If we find the key, remove the mapping and we're done!
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) {
      setPair(S, map, cons(
        Bool(false),
        node.right));
      return true;
    }
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveDelete(S, side, key, bits >> 1);
}

static value_t mapDelete(state_t* S, value_t map, value_t key) {
  if (map.type != MAP) return Bool(false);
  return Bool(recursiveDelete(S, map, key, hash(key)));
}

static value_t recursiveRead(state_t* S, value_t map, value_t key, int32_t bits) {
  pair_t node = getPair(S, map);
  // If we find the key, return the value!
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) return mapping.right;
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return Bool(false);
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  if (side.type == PAIR) return recursiveRead(S, side, key, bits >> 1);
  return Bool(false);
}

static value_t mapRead(state_t* S, value_t map, value_t key) {
  if (map.type != MAP) return Bool(false);
  return recursiveRead(S, map, key, hash(key));
}

static value_t RawBuffer(state_t* S, type_t type, int32_t length, const uint8_t* data) {
  if (data && length < 0) {
    length = (int32_t)strlen((char*)data);
  }
  if (length + 4 > S->num_bytes - S->next_bytes) {
    // TODO: GC and/or resize space
    printf("ERROR: not enough space to allocate new buffer\n");
    return Bool(false);
  }
  int32_t start = S->next_bytes;
  buffer_t* buf = (buffer_t*)(S->bytes + start);
  buf->gc = true;
  buf->length = length;
  if (data) {
    memcpy(buf->data, data, (size_t)length);
  }
  else {
    memset(buf->data, 0, (size_t)length);
  }
  S->next_bytes += length + 4;
  return (value_t){
    .gc = true,
    .type = type,
    .value = start
  };
}

static value_t String(state_t* S, int32_t len, const uint8_t* str) {
  return RawBuffer(S, STRING, len, str);
}

static value_t Symbol(state_t* S, int32_t len, const uint8_t* str) {
  if (len < 0) {
    len = (int32_t)strlen((const char*)str);
  }
  uint8_t* offset = S->bytes;
  while (true) {
    buffer_t* buf = (buffer_t*)offset;
    // If we reach the end, it's not here, just add a new buffer.
    if (offset > S->bytes + S->num_bytes || !(buf->gc || buf->length)) {
      return RawBuffer(S, SYMBOL, len, (uint8_t*)str);
    }
    // If we find a matching string, reuse the existing value.
    if (buf->gc && buf->length == len && !strcmp((char*)buf->data, (const char*)str)) {
      return (value_t){
        .gc = 1,
        .type = SYMBOL,
        .value = (int32_t)(offset - S->bytes)
      };
    }
    offset += buf->length + 4;
  }
}

static value_t Buffer(state_t* S, int32_t length, const uint8_t* data) {
  return RawBuffer(S, BYTE_ARRAY, length, data);
}

static value_t Pixels(state_t* S, int32_t length, const uint32_t* data) {
  if (length < 0) return Bool(false);
  return RawBuffer(S, FRAME_BUFFER, length << 3, (const uint8_t*)data);
}

static buffer_t* getBuffer(state_t* S, value_t value) {
  return (buffer_t*)(S->bytes + value.value);
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

static void printMapNode(state_t* S, value_t node, bool later) {
  if (falsy(node)) return;
  pair_t pair = getPair(S, node);
  if (pair.left.type == PAIR) {
    if (later) putchar(' ');
    pair_t mapping = getPair(S, pair.left);
    prettyPrint(S, mapping.left);
    putchar(':');
    prettyPrint(S, mapping.right);
    later = true;
  }
  node = pair.right;
  if (falsy(node)) return;
  pair_t split = getPair(S, node);
  printMapNode(S, split.right, later);
  printMapNode(S, split.left, later);
}


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
    case STRING: {
      buffer_t* buf = getBuffer(S, value);
      printf("\"%*s\"", buf->length, buf->data);
      break;
    }
    case SYMBOL: {
      buffer_t* buf = getBuffer(S, value);
      printf(":%*s", buf->length, buf->data);
      break;
    }
    case BYTE_ARRAY: {
      buffer_t* buf = getBuffer(S, value);
      putchar('<');
      for (int i = 0; i < buf->length; i++) {
        if (i) putchar(' ');
        printf("%02x", buf->data[i]);
      }
      putchar('>');
      break;
    }
    case FRAME_BUFFER: {
      buffer_t* buf = getBuffer(S, value);
      uint32_t* data = (uint32_t*)buf->data;
      int32_t len = buf->length >> 3;
      putchar('<');
      for (int i = 0; i < len; i++) {
        if (i) putchar(' ');
        printf("%08x", data[i]);
      }
      putchar('>');
      break;
    }
    case PAIR: {
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
      putchar('|');
      printSetNode(S, value, false);
      putchar('|');
      break;
    case MAP:
      putchar('{');
      printMapNode(S, value, false);
      putchar('}');
      break;
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

void testValues() {
  deadbeef_srand(42);
  assert(sizeof(value_t) == 4);
  assert(sizeof(pair_t) == 8);
  state_t* S = State();
  for (int64_t i = 1; i > 0 && i <= INT64_MAX; i <<= 4) {
    printf("Testing %"PRId64"\n", i);
    dump(S, Integer(S, i));
    dump(S, Integer(S, -i));
  }
  dump(S, Integer(S, 0));
  dump(S, Integer(S, 1));
  dump(S, Integer(S, -1));
  dump(S, Bool(true));
  dump(S, Bool(false));
  dump(S, Pair(S, cons(
    Integer(S, 1),
    Integer(S, 2))));
  dump(S, Rational(S, 1, 2));
  for (int i = 0; i < 100; i++) {
    dump(S, Char(i));
  }
  dump(S, Char('@'));
  dump(S, Char(9654));   // ▶
  dump(S, Char(128513)); // 😁
  dump(S, Char(128525)); // 😍

  printf("\n--STACKS--\n");
  printf("Creating an empty stack -> ");
  value_t stack = Stack(S);
  dump(S, stack);
  printf("Pushing 'A' onto the stack -> ");
  dump(S, stackPush(S, stack, Char('A')));
  dump(S, stack);
  printf("Pushing 'B' onto the stack -> ");
  dump(S, stackPush(S, stack, Char('B')));
  dump(S, stack);
  printf("Pushing 'C' onto the stack -> ");
  dump(S, stackPush(S, stack, Char('C')));
  dump(S, stack);
  printf("Peeking on the stack -> ");
  dump(S, stackPeek(S, stack));
  printf("Reading stack length -> ");
  dump(S, stackLength(S, stack));
  printf("Popping from the stack -> ");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the stack -> ");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the stack -> ");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Popping from the empty stack -> ");
  dump(S, stackPop(S, stack));
  dump(S, stack);
  printf("Peeking on the empty stack -> ");
  dump(S, stackPeek(S, stack));
  printf("Reading empty stack length -> ");
  dump(S, stackLength(S, stack));

  printf("\n--SETS--\n");
  printf("Creating ane empty set -> ");
  value_t set = Set(S);
  dump(S, set);
  for (int i = 0; i < 10; i++) {
    int n = deadbeef_rand() % 20;
    printf("set.add(%d) -> ", n);
    dump(S, setAdd(S, set, Int(n)));
    dump(S, set);
    n = deadbeef_rand() % 20;
    printf("set.del(%d) -> ", n);
    dump(S, setRemove(S, set, Int(n)));
    dump(S, set);
    for (int j = 0; j < 20; j++) {
      putchar(truthy(setHas(S, set, Int(j))) ? '#' : '_');
    }
    putchar('\n');
  }
  printf("Adding a pair to the set -> ");
  dump(S, setAdd(S, set, Pair(S, cons(Bool(true),Char('b')))));
  dump(S, set);
  printf("Adding a rational to the set -> ");
  dump(S, setAdd(S, set, Rational(S, 1, 3)));
  dump(S, set);

  printf("\n--MAPS\n");
  printf("Creating an empty map: ");
  value_t map = Map(S);
  dump(S, map);
  printf("Setting mapping from 10 to 20 -> ");
  dump(S, mapSet(S, map, Int(10), Int(20)));
  dump(S, map);
  printf("Setting mapping from 'A' to 42 -> ");
  dump(S, mapSet(S, map, Char('A'), Int(42)));
  dump(S, map);
  printf("Setting mapping from 'B' to -5 -> ");
  dump(S, mapSet(S, map, Char('B'), Int(-5)));
  dump(S, map);
  printf("Setting mapping from 'A' to 0 -> ");
  dump(S, mapSet(S, map, Char('A'), Int(0)));
  dump(S, map);
  printf("Setting mapping from 'A' to 0 -> ");
  dump(S, mapSet(S, map, Char('A'), Int(0)));
  dump(S, map);
  printf("Deleting mapping from 'A' -> ");
  dump(S, mapDelete(S, map, Char('A')));
  dump(S, map);
  printf("Deleting mapping from 10 -> ");
  dump(S, mapDelete(S, map, Int(10)));
  dump(S, map);
  printf("Setting mapping from 'A' to 100 -> ");
  dump(S, mapSet(S, map, Char('A'), Int(100)));
  dump(S, map);
  printf("Setting mapping from 10 to 20 -> ");
  dump(S, mapSet(S, map, Int(10), Int(20)));
  dump(S, map);
  printf("Reading mapping from 'A' -> ");
  dump(S, mapRead(S, map, Char('A')));
  printf("Reading mapping from 10 -> ");
  dump(S, mapRead(S, map, Int(10)));
  printf("Reading mapping from 20 -> ");
  dump(S, mapRead(S, map, Int(20)));
  for (int i = 0; i < 10; i++) {
    int k = deadbeef_rand() % 20;
    int v = deadbeef_rand() % 20;
    printf("map.set(%d, %d) -> ", k, v);
    dump(S, mapSet(S, map, Int(k), Int(v)));
    dump(S, map);
    k = deadbeef_rand() % 20;
    printf("map.delete(%d) -> ", k);
    dump(S, mapDelete(S, map, Int(k)));
    dump(S, map);
  }

  value_t history = Stack(S);
  stackPush(S, history, Char('A'));
  stackPush(S, history, Char('B'));
  stackPush(S, history, Char('B'));
  stackPush(S, history, Char('A'));
  dump(S, history);

  value_t check = Stack(S);
  stackPush(S, check, Char('A'));
  stackPush(S, check, Char('B'));
  stackPush(S, check, Char('B'));
  stackPush(S, check, Char('A'));

  dump(S, stackIs(S, history, check));

  stackPush(S, check, Char('Z'));
  dump(S, check);
  dump(S, stackIs(S, history, check));
  stackPush(S, history, Char('Y'));
  dump(S, history);
  dump(S, stackIs(S, history, check));
  stackPop(S, check);
  dump(S, check);
  dump(S, stackIs(S, history, check));
  stackPop(S, history);
  dump(S, history);
  dump(S, stackIs(S, history, check));
  stackPush(S, history, Char('O'));
  stackPush(S, history, Char('P'));
  dump(S, history);

  dump(S, stackReverse(S, history));

  dump(S, String(S, -1, (const uint8_t*)"Hello World"));
  dump(S, Symbol(S, -1, (const uint8_t*)"name"));
  dump(S, Buffer(S, 5, 0));
  dump(S, Buffer(S, 4, (uint8_t[]){0xde,0xad,0xbe,0xef}));
  dump(S, Pixels(S, 8, 0));
  dump(S, Pixels(S, 4*8, (uint32_t[]){
    OFF, ORN, YEL, OFF,
    PUR, RED, ORN, YEL,
    BLU, PUR, RED, ORN,
    GRN, BLU, PUR, RED,
    YEL, GRN, BLU, PUR,
    ORN, YEL, GRN, BLU,
    RED, ORN, YEL, GRN,
    OFF, RED, ORN, OFF,
  }));
  printf("Testing equality of two symbols with same contents: ");
  dump(S, Bool(eq(Symbol(S, -1, (const uint8_t*)"test"), Symbol(S, -1, (const uint8_t*)"test"))));
  printf("Testing equality of two strings with same contents: ");
  dump(S, Bool(eq(String(S, -1, (const uint8_t*)"test"), String(S, -1, (const uint8_t*)"test"))));
}

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

static uint8_t* code = (uint8_t[]){
  LitTrue,
  LitInt, Int7(25),
  LitInt, Int7(-25),
  LitInt, Int14(42),
  LitInt, Int14(-42),
  LitInt, Int21(5000),
  LitInt, Int21(-5000),
  LitRational, Int21(-5000), Int14(111),
  LitChar, Int14('%'),
  LitString, Int7(11), 'H','e','l','l','o',' ','W','o','r','l','d',
  LitSymbol, Int7(4), 'n','a','m','e',
  LitBuffer, Int21(4*4), Uint32(RED),Uint32(BLU),Uint32(ORN),Uint32(YEL),
  LitPixels, Int21(4), Uint32(RED),Uint32(BLU),Uint32(ORN),Uint32(YEL),
  LitPair, LitTrue, LitFalse,
  LitStack, Int7(3), LitTrue, LitInt, Int7(10), LitChar, Int14('%'),
  LitSet, Int7(3), LitTrue, LitChar, Int14('%'), LitChar, Int14('*'),
  LitMap, Int7(2),
    LitSymbol, Int7(4), 'n','a','m','e',
      LitString, Int7(3), 'T','i','m',
    LitSymbol, Int7(3), 'a','g','e',
      LitInt, Int14(33),
  Block, Int7(2), LitTrue, LitFalse,
  LitFalse,
};

uint8_t* readInt(uint8_t* pc, int64_t* out) {
  *out = 0;
  int64_t extend = *pc & 0x40 ? ~0 : 0;
  do {
    extend <<= 7;
    *out = (*out << 7) | (*pc & 0x7f);
  } while (*pc++ & 0x80);
  *out |= extend;
  return pc;
}

uint8_t* eval(state_t* S, uint8_t* pc, value_t* out) {
  int64_t num, num2;
  switch ((opcode_t)*pc++) {
    case LitFalse:
      *out = Bool(false);
      return pc;
    case LitTrue:
      *out = Bool(true);
      return pc;
    case LitInt: // num
      pc = readInt(pc, &num);
      *out = Integer(S, num);
      return pc;
    case LitRational: // num num
      pc = readInt(pc, &num);
      pc = readInt(pc, &num2);
      *out = Rational(S, num, num2);
      return pc;
    case LitChar: // num
      pc = readInt(pc, &num);
      *out = Char((int32_t)num);
      return pc;
    case LitString: // len:num byte*
      pc = readInt(pc, &num);
      *out = String(S, (int32_t)num, pc);
      return pc + num;
    case LitSymbol: // len:num byte*
      pc = readInt(pc, &num);
      *out = Symbol(S, (int32_t)num, pc);
      return pc + num;
    case LitBuffer: // len:num byte*
      pc = readInt(pc, &num);
      *out = Buffer(S, (int32_t)num, pc);
      return pc + num;
    case LitPixels: // len:num byte*
      pc = readInt(pc, &num);
      *out = Pixels(S, (int32_t)num, (uint32_t*)pc);
      return pc + num * 4;
    case LitPair: { // value value
      value_t left, right;
      pc = eval(S, pc, &left);
      pc = eval(S, pc, &right);
      *out = Pair(S, cons(left, right));
      return pc;
    }
    case LitStack: // len:num value*
      pc = readInt(pc, &num);
      *out = Stack(S);
      for (int i = 0; i < num; i++) {
        value_t value;
        pc = eval(S, pc, &value);
        stackPush(S, *out, value);
      }
      return pc;
    case LitSet: // len:num value*
      pc = readInt(pc, &num);
      *out = Set(S);
      for (int i = 0; i < num; i++) {
        value_t value;
        pc = eval(S, pc, &value);
        setAdd(S, *out, value);
      }
      return pc;
    case LitMap: // len:num (value/value)*
      pc = readInt(pc, &num);
      *out = Map(S);
      for (int i = 0; i < num; i++) {
        value_t key;
        value_t value;
        pc = eval(S, pc, &key);
        pc = eval(S, pc, &value);
        mapSet(S, *out, key, value);
      }
      return pc;
    case Block: // len:num value*
      pc = readInt(pc, &num);
      for (int i = 0; i < num; i++) {
        pc = eval(S, pc, out);
      }
      return pc;
  }
  return pc;
}

int main() {
  value_t value;
  uint8_t* pc = code;
  state_t* S = State();
  while (*pc) {
    pc = eval(S, pc, &value);
    prettyPrint(S, value);
    putchar('\n');
  }
  return 0;
}
