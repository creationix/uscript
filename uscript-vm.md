# Memory Layout

- Program Space: maximum of 2^28 8-bit bytes. (256Mb)
- Object Heap: maximum of 2^13-2 32-bit words. (about 32Kb)
- Buffer Heap: maximum of 2^28 at 32-bit alignment. (1Gb)

If we add a virtual memory mapper, flash can be used to reach the higher
addressing limits on a microcontroller's small ram, but large flash.

For example, we could mount an SD card and read a large audio file in 512Kb
chunks.  Even without a SD card interface, most the chips we target have much
larger flash sizes than ram.

## Garbage Collector

There are two heap sections.  One is for fixed-size heap objects that contains
all the boxed types (including headers for byte arrays).
The second section is for storing the contents of byte arrays.

The garbage collector will perform a mark and sweep using an algorithm based
on the picobit GC.

A mark/sweep will be triggered by heap address wrapping around or by a
compaction being requested.

A compaction will compact all byte buffers using reverse pointers in the head
to update the pointers in the buffer heap objects.

A compaction is triggered in case of out of memory errors while allocating a new
buffer.

The language will allow users to trigger full compaction or just mark/sweep GCs
as well.

## Types

- Boolean is simply `true` or `false`.

- Number is any signed 28-bit integer (-134,217,728 to 134,217,727)

- Pair is a data structure that holds two other values.  From this primitive
  linked lists and maps can be created.

- Buffer is a mutable, fixed-length byte array up to 65536 bytes long.

## Value Encoding

Values are 14 bits long (to fit inside boxed heap values).

The first two values are the boolean literals `false` and `true`.

- 0: `false`
- 1: `true`

Then the next 8190 values are heap pointers.

- 2-8191: Object heap pointer offset. (0 to 8189)

The remaining half of the space is for unboxed integer literals.

- 8192-16383: the lower 13 bits are a signed integer (range -4096 to 4095)

## Object Heap Encoding

Heap boxes are all 32 bits wide, the first 4 bits hold type and GC state.

There are currently 3 types of heap boxes, 4 reserved prefixes and the free
marker:

- Integer  000G int28_t (range from -134,217,728 to 134,217,727)
- Buffer   001G uint28_t (byte heap offset in 4-byte word increments)
- Pair     01GG (value) (value)
- Reserved 100G
- Reserved 101G
- Reserved 110G
- Reserved 111G (must have at least one 0 in body)
- Free     11111111 11111111 11111111 11111111

A `next` pointer is kept globally to start looking after the previous allocation
when looking for a new empty space.  When the end is reached and the pointer
wraps around, a sweep and collect GC is triggered.  If the end is reached again,
a fatal out-of-memory error is triggered.

## Byte Heap Encoding

Allocations in the byte heap are 4-byte aligned, but contain data of any byte
length.

- bbbbbbbbbbbbb lllllllllllllllllll [data...]

Each allocation has a 4 byte header.  The first 13 bits are a back-reference
to the heap object that points here.  This is used to update references during
compaction.  When a buffer object is freed by the sweeper, the corresponding
byte array is marked as free by clearing the back-reference.

The remaining 19 bits are used for length meaning buffers can be up to 512Kb
long.

A `next` pointer is kept globally here as well.  When the end is reached, a
compaction is triggered.  The pointer is then moved to the end of the compacted
space. If there is still no room, a fatal out-of-memory error happens.

While compaction is somewhat expensive, allocation very fast normally.

## Program Space

ByteCode is composed of opcodes and literal values.  Bytecode itself is a linear
stream of data, but is interpreted as an abstract syntax tree by the
interpreter.

The entire system image is compiled/flashed at once so that inter-module calls
can jump directly to code offsets and symbols can compile down to globally
unique integers.  Since this memory space read-only, no GC is used and it can
be stored on flash if memory is low.

## Expression Opcodes

Opcodes consume one byte and generally have a fixed number of arguments they
consume, thus allowing the parser/interpreter to not need arity metadata.  The
exception is the integer opcode which has up to 4 bytes of extra data (7 bits
each) using the first bit as a "more" flag.

### Literals

- FALSE, TRUE
- VARINT(...) - Followed by base-128 big-endian variable-length integer.
- PAIR(left, right)
- BUFFER(len)

### Integer Operators

- ADD(a,b), SUB(a,b), MUL(a,b), DIV(a,b), MOD(a,b), NEG(a)
- BNOT(a), BXOR(a,b), BAND(a,b), BOR(a,b), LSHIFT(a,b), RSHIFT(a,b)
- GT(a,b), GTE(a,b), LT(a,b), LTE(a,b), EQ(a,b), NEQ(a,b)
- SRAND(seed), RAND(modulus)

### Logical Operators

- AND(a,b), OR(a,b), XOR(a,b), NOT(a)
- CHOOSE(condition,a,b)

`AND` and `OR` are logical short-circuit and don't always evaluate sub-expressions.
`CHOOSE` takes a condition and two values and chooses the first on true

### Buffer Operators

- LEN(buff) -> length
- PEEK(buff, offset) -> byte
- POKE(buff, offset, byte)
- FILL(buff, start, end, byte)
- COPY(source, dest, sourceOffset, destOffset, length)
- SUB(buff, start, end) -> buff copy
- INDEX(buff, byte) -> offset
- FIND(buff, searchBuff) -> offset

### Pair Operators

- HEAD(pair) -> head
- TAIL(pair) -> tail
- SETHEAD(pair, value)
- SETTAIL(pair, value)

Lists/Sets are made using nested pairs:

    (1 (2 (3 (4 false))))

- LLEN(pair) -> length - Count number of values in list.
- LGET(pair, index) -> value - Walk to index and return value.
- LSET(pair, index, value) - Walk to index and replace value.
- LDEL(pair, index) - Remove value by index.

- SHAS(pair, value) -> has - Check if set has a value.
- SDEL(pair, value) -> had - Remove value by value.
- SADD(pair, value) -> added - Add value to list if not already there.

Maps are made using pairs inside a list:

    ((k v) ((k v) ((k v) -)))

- MSET(pair, key, value) -> added - Add or replace value in map by key.
- MGET(pair, key) -> value - Find value by key.
- MDEL(pair, key) -> value - Remove value by key.

### Digital I/O

We have the basic GPIO functions that arduino users love.

- MODE(pin, mode)
- WRITE(pin, bool) - Digital Write
- READ(pin) -> bool - Digital Read
- PWRITE(pin, value) - PWM Write
- AREAD(pin) -> value - Analog Read

Also we have I2C primitives for implementing protocols easily.

- ISETUP(pinSDA, pinSCL, speed) -> i2c
- ISTART(i2c) - Send a start sequence.
- ISTOP(i2c) - Send a stop sequence.
- IADDR(i2c, addr, direction) - Send address and direction.
- IWRITE(i2c, byte)
- IREAD(i2c) -> byte
- IAWRITE(i2c, buffer)
- IAREAD(i2c, len) -> buffer

### Local Variables

- ALLOC(n) - UnShift n slots into local space.
- FREE(n) - Shift out n slots from local space.
- SET(i, value) - Store a value in the variable stack.
- GET(i) -> value - Load a value from the stack.
- RESULT -> value - Load the return value from the last function call.

## Statement OpCodes

Statements are like expressions, but they may not be used as part of an
expression tree, but must be top-level in a block.  This keeps the amount of
stated needed to suspend a thread minimal while keeping fast recursive
expression trees in the interpreter.

### Pausers

- DELAY(ms)
- UDELAY(us)
- WAIT(condition)
- YIELD
- GC(compact)

These all yield control to other threads till a condition is true.  

## Control Flow

- IF(condition) body END
- SKIP body END

SKIP is used to implement if/elseif/else, but acts like a skip

IF (LT (RAND 10) 5) ... SKIP ... END

IF (cond) ... SKIP IF (cond) ... SKIP ... END END

## Loops

- WHILE(condition) body LOOP
- DOWHILE(condition) body LOOP
- BREAK, CONTINUE

DOWHILE is like WHILE, except it skips the condition on first run.

There is a loop stack that gets serialized to a list when suspended.

Loops simply push the PC to the condition head.

BREAK will skip to past LOOP.

CONTINUE will act like LOOP and jump back.

## Iterators

- FOR (start, end, step) body LOOP
- EACH (list) body LOOP

Iterators shift in a variable slot for the iteration variable.

Their entry in the loop stack is more complex.

For the `FOR` iterator, it stores (PC (step end))

For the `EACH` iterator, it stores (PC, tail)

## Function Calls

- CALL(pc) - Jump to a new place in the code and return when done.
- RETURN(value) - Unwind the callstack and return to the caller storing a value
  in the return value register.

Since all modules share the same value stack for local variables, arguments are
passed by convention by having them at the top of the stack.
