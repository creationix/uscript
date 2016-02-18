#include "src/uscript.h"
#include "src/dump.h"
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#define RED 0xff2200
#define ORN 0xf08000
#define YEL 0xe8e800
#define GRN 0x22ee00
#define BLU 0x0088ff
#define PUR 0x9000f0
#define OFF 0x000000


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
  dump(S, Pair(S,
    Integer(S, 1),
    Integer(S, 2)));
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
  dump(S, setAdd(S, set, Pair(S, Bool(true),Char('b'))));
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
      *out = Pair(S, left, right);
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
