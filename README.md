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

## User Code

- def [name] [expression] - define function
- rm [name] - delete function
- call [name] - call function (blocking) (proper tail call support)
- run [name] - put function in event queue (non-blocking)
- return [expression] - exit function early with value

## Events

- wait [expression] - wait for expression to be 1 (blocking)
- on [expression] [name] - call function when expression is 1 (non-blocking)

## Timers

- delay [expression] - delay for ms (blocking)
- timer [expression] [name] - run function after ms (non-blocking)

## Variables

- set [variable] [expression] - store value
- get [variable] - get value
- incr [variable] - increment variable by 1 (++i)
- decr [variable] - decrement variable by 1 (--i)

## Control Flow

- if [expression] [expression] elif [expression] [expression] else [expression]
- match [expression] with [expression] [expression] else [expression]
- while [expression] [expression] - conditional loop
- do [num] [expression]* - create a block of multiple expressions
- loop [num] [expression]* - unconditional loop
- break [expression] - exit while/loop with value

## Logic

- not [expression] - logical not
- and [expression] [expression] - logical and with short-circuit and value preservation
- or  [expression] [expression] - logical or with short-circuit and value preservation
- xor [expression] [expression] - logical exclusive or

## Comparison

- eq  [expression] [expression] - equal
- neq [expression] [expression] - not equal
- gte [expression] [expression] - greater than or equal
- lt  [expression] [expression] - less than

## Arithmetic

- neg [expression] - negate value
- add [expression] [expression] - add
- sub [expression] [expression] - subtract
- mul [expression] [expression] - multiply
- div [expression] [expression] - divide
- mod [expression] [expression] - mod

## Random Library

 - rand [expression] - Random number between 0 and val - 1

## Pin Library

- pm [pin] [expression] - pin mode
- dw [pin] [expression] - digital write
- pw [pin] [expression] - pwm write
- dr [pin] - digital read
- ar [pin] - analog read

## Neopixel Library

 - np [pin] [num] - setup neopixel strip
 - npw [pin] [idx] [rgb] - set red/green/blue pixel from r << 16 | g << 8 | b
 - rgb [r] [g] [b] - combine red/green/blue into rgb integer
 - hsv [h] [s] [v] - convert hue/saturation/value to rgb
 - hcl [h] [c] [l] - convert luma/chroma/hue pixel to rgb
 - update [pin] - send update


## Servo Library

 - servo [pin] [duty] [freq] - Setup servo
 - move [pin] [duty] - update duty cycle

## Tone library

 - tone [pin] [freq] [duration]

# Misc

## Constraints

 - 26 local variables a-z containing 32-bit signed integers
 - name -> index -> offset - user function mapping (use index in ast)
 - enum -> index - builtin function mapping (modules are ifdef blocks)

## Constants

 - T / ON / OUT - constants for 1
 - F / OFF / IN - constants for 0

## Assembly Syntax

Here is a sample program that toggles pin 13 every second:

```uscript
; define a function that toggles pin 13
func t13
  dw 13
    not dr 13

; set pin 13 to output mode
pm 13 OUT
; run the t13 code every 1000 ms
start 1000 t13
```

## Binary Format



```
[func] [t13] [dw] 13 [not] [dr] 13 [pm] 13 OUT [start] 1000 [t13]
```

## Watch expressions

The `wait` and `on` statements accept an expression and wait for it to be 1.
This is done without resorting to busy-poll by doing a dependency graph analysis
and getting all the possible changeable state that could affect the expression.
This includes pins, variables, and functions.

Pins will busy-poll under the hood (once per active pin) if the hardware doesn't
support interrupts.  On top of this abstraction, they will appear event based.

Variables will have hooks in the code on set to be event based.

Functions will have hooks in function definition and deletion.  Also function
contents will be scanned recursively for state dependencies. The graph will be
re-calculated if a function is modified.

It's best to not use functions that have side-effects in this expression as
they will be called whenever a change happens and the result needs to be tested.

[bitlash]: http://bitlash.net/
[web assembly]: https://github.com/WebAssembly/design/blob/master/README.md
