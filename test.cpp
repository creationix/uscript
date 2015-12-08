#include <assert.h>
#include "vm/vm.cpp"



int alive = 0;
int isAlive() {
  return true;
  // return --alive > 0;
}

intptr_t stack[100];
intptr_t value;

// Use for numbers between 64 and 8,191
#define num(n) (0x40|(n)>>7),((n)&0x7f)

int main() {
  // -- A non-recursive fibbonacci generator, finds the 92nd value.
  // -- should be 7540113804746346429.
  // do
  //   var count 92
  //   var a 0
  //   var b 1
  //   while count do
  //       decr count
  //       var c add a b
  //       set a b
  //       set b c
  //     end
  //   a
  // end
  eval(stack, (uint8_t[]){Do,
    Set, 0, num(92), // count = x
    Set, 1, 0, // a = 0
    Set, 2, 1, // b = 1
    While, Get, 0, Do,
        Decr, 0,
        Set, 3, // c = a + b
          Add, Get, 1, Get, 2,
        Set, 1, Get, 2, // a = b
        Set, 2, Get, 3, // b = c
      End,
    Get, 1,
  End}, &value);
  assert(value == 7540113804746346429);

  // -- A non-recursive exponential generator, finds the 25th value.
  // -- should be 7034535277573963776.
  // do
  //   var count = 5
  //   var a = 1
  //   while gt count 1
  //     do
  //       decr count
  //       set a
  //         mul a count
  //     end
  //   a
  // end
  eval(stack, (uint8_t[]){Do,
    Set, 0, 25, // count
    Set, 1, 1, // a
    While, Gt, Get, 0, 1,
      Do,
        Set, 1,
          Mul, Get, 1, Get, 0,
        Decr, 0,
      End,
      Get, 1,
  End}, &value);
  assert(value == 7034535277573963776);

  eval(stack, (uint8_t[]){Do,
    Srand, 0,
    Dump, Rand, num(8000),
    Dump, Rand, num(8000),
    Dump, Rand, num(8000),
    Dump, Rand, num(8000),
    Dump, Rand, num(8000),

    Set, 0, 0, // seed
    Set, 1, // beef
      0x0d|0x40,
      0x75|0x80,
      0x36|0x80,
      0x7d|0x80,
      0x6f, // 0xdeadbeef
    Set, 2, 0, // arg
    Gosub, 0x00, 0x18,
    Set, 2, num(8000),
    Dump, Gosub, 0x00, 0x1e,
    Dump, Gosub, 0x00, 0x1a,
    Dump, Gosub, 0x00, 0x16,
    Dump, Gosub, 0x00, 0x12,
    Dump, Gosub, 0x00, 0x0e,
    Return,
    Do,
      Set, 0, Get, 2,
      Set, 1, // beef
        0x0d|0x40,
        0x75|0x80,
        0x36|0x80,
        0x7d|0x80,
        0x6f, // 0xdeadbeef
    End,
    Do,
      Set, 0, Bxor,
        Lshift, Get, 0, 7,
        Add,
          Rshift, Get, 0, 25,
          Get, 1,
      Set, 1, Bxor,
        Lshift, Get, 1, 7,
        Add,
          Rshift, Get, 1, 25,
          0x0d|0x40,
          0x75|0x80,
          0x36|0x80,
          0x7d|0x80,
          0x6f, // 0xdeadbeef
      Mod, Get, 0, Get, 2,
    End,
  End}, &value);

  // printf("\n\n");
  //
  // alive = 3;
  // eval(stack, (uint8_t[]){Call, 0, Func, Do,
  //   Gset, 0, Func, Do,
  //     Mode, D7, 1,
  //     Write, D7, 0,
  //     Forever, Do,
  //       Delay, num(333),
  //       Write, D7,
  //         Not, Read, D7,
  //     End,
  //   End,
  //   Call, 0, Gget, 0,
  // End}, &value);
  //
  // printf("\n\n");
  //
  // eval(stack, (uint8_t[]){Call, 0, Func, Do,
  //   Call, (16 / sizeof(intptr_t)), Gget, 0,
  //   Gset, 0, Free, Gget, 0,
  // End}, &value);

  return 0;
}

buffer_t* copyBuffer(int len, uint8_t* data) {
  buffer_t* buf = (buffer_t*)malloc(sizeof(buffer_t) + len);
  memcpy(buf->data, data, len);
  return buf;
}
