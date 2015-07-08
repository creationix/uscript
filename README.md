# μScript Programming Environment

μScript is a simple programming engine for μControllers co-designed by a 9-year-old.

The goal is to enable rapid prototyping and learning with hardware and software.

# Abstract Syntax Tree Opcodes

The core language is inspired by [bitlash][] and the idea of using AST as
byte-code from [web assembly][].  Instead of storing textual source code on the
micro-controller, a compact binary representation of the AST is stored locally.
It's assumed that some external compiler or tool will send the pre-compiled
binary code to the device via serial, wifi, ethernet, bluetooth, radio, or some
other means.

The system is not static at all though, it will still have a repl and be able to
modify it's code live without re-flashing or rebooting.

The builtins and user functions are all resolved to 8-bit index values
shown in symbolic form here.

## Primitives

- [number] - 32-bit signed integer, var-length encoded

The number type is special and is variable length
encoded.  For example `13` is just the byte `0 0 001101`, but `1000` is two bytes
`0 1 101000 0 0001111`.

If the first bit of a byte is low, it's the start of a variable-length integer.
The remaining 7 bits and all 8 bits in each digit after that consist of a more
bit and data bits.  The first byte contains 6 bits of data and each after that
contains 7 bits.  The least significant bits come first.

## Variables

- SET [variable] [expression] - store value
- GET [variable] - get value
- INCR [variable] [expression] - increment variable by expression
- DECR [variable] [expression] - decrement variable by expression

## Stack

 - READ [num] - read a value on the stack without removing it. (0 is top)
 - WRITE [num] [expression] - replace a value on the stack
 - INSERT [num] [expression] - insert a new value on the stack (0 is push)
 - REMOVE [num] - remove an item from the stack (0 is pop)

## Control Flow

- IF [expression] [expression] ELIF [expression] [expression] ELSE [expression]
- MATCH [expression] WHEN [expression] [expression] ELSE [expression]
- WHILE [expression] [expression] - conditional loop
- DO [num] [expression]* - create a block of multiple expressions
- FOR [variable] [expression] [expression] [expression] - Iterate variable from
   first expression to second expression evaluating third expression every time.
- WAIT [expression] - Block until an expression is true. Returns expression.

## Logic

- NOT [expression] - logical not
- AND [expression] [expression] - logical and with short-circuit and value preservation
- OR  [expression] [expression] - logical or with short-circuit and value preservation
- XOR [expression] [expression] - logical exclusive or

## Bitwise Operations

- BNOT   [expression] - bitwise not
- BAND   [expression] [expression] - bitwise and
- BOR    [expression] [expression] - bitwise or
- BXOR   [expression] [expression] - bitwise exclusive or
- LSHIFT [expression] [expression] - left shift
- RSHIFT [expression] [expression] - right shift

## Comparison

- EQ  [expression] [expression] - equal
- NEQ [expression] [expression] - not equal
- GTE [expression] [expression] - greater than or equal
- LTE [expression] [expression] - less than or equal
- GT  [expression] [expression] - greater than
- LT  [expression] [expression] - less than

## Arithmetic

- NEG [expression] - negate value
- ADD [expression] [expression] - add
- SUB [expression] [expression] - subtract
- MUL [expression] [expression] - multiply
- DIV [expression] [expression] - integer divide
- MOD [expression] [expression] - modulus
- ABS [expression] - absolute value

## Misc

- DELAY [expression] - Block for given ms.
- RANDOM [expression] - Return a number between 0 and value - 1.
- PRINT [expression] - Print result.

## Saved Expressions

Stubs are subroutines.  They don't have parameters, but do share the same 26
global variables as the rest of the VM.  You get 26 slots for storing them.
The definitions are stored in EEPROM or some other persistent storage.

- DEF [variable] [expression] - store an expression
- RM [variable] - delete a stored expression
- RUN [variable] - run a stored expression

## Wireing

Currently only works for microcontrollers, but soon raspberry pi too.

- PM [expression] [expression] - pinMode
- DW [expression] [expression] - digitalWrite
- AW [expression] [expression] - analogWrite
- DR [expression] - digitalRead
- AR [expression] - analogRead

## Constraints

 - 26 local variables a-z containing 32-bit signed integers (64-bit on desktop builds.)

## Constants

 - a0, a1, aN in arduino mode for specifying analog pins.

## Assembly Syntax

Here is a sample program that toggles pin 13 every second:

```uscript-asm
PM 13 1
WHILE 1
  DW NOT DR 13
  DELAY 1000
```

[bitlash]: http://bitlash.net/
[web assembly]: https://github.com/WebAssembly/design/blob/master/README.md
