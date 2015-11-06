# Types

- Number, Boolean, Buffer, Cons, Symbol, Closure, Continuation, PC

# Expressions

Expressions form a tree and are not interruptable.

## Literals

- integer literal, boolean literal, symbol literal

## Integer Operators

- ADD, SUB, MUL, DIV, MOD, NEG
- BNOT, BXOR, BAND, BOR, LSHIFT, RSHIFT
- GT, GTE, LT, LTE, EQ, NEQ
- SRAND, RAND

## Logical Operators

- AND, OR, XOR, NOT
- CHOOSE

`AND` and `OR` are logical short-circuit and don't always evaluate sub-expressions.
`CHOOSE` takes a boolean and two values and chooses the first on true

## Buffer Operators

- NEW, PEEK, POKE, LEN, FILL, COPY

`NEW` Creates a new buffer of fixed length.

## Cons Operators

- PAIR, HEAD, TAIL, SETHEAD, SETTAIL

`PAIR` takes two arguments and returns a cons cell

Lists/Sets are made using nested pairs:

    (1 (2 (3 (4 -))))

- LGET, LSET, LLEN
- SHAS, SDEL, SADD

Maps are made using pairs inside a list:

    ((k v) ((k v) ((k v) -)))

- MSET, MGET, MHAS, MDEL, MLEN

## Digital I/O

- MODE, WRITE, READ, PWRITE, AREAD
- ISETUP, ISTART, ISTOP, IADDR, IWRITE, IREAD

## Sliding Variables Stack

- ALLOC, FREE, GET, SET

`ALLOC` shifts in n spaces to the local variables space
`FREE` unshifts out n spaces

# Statements

## Pausers

- DELAY, UDELAY, WAIT, YIELD

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

- RUN, DONE

Jump to another part of the program and return on DONE

# Samples

```js
function main() {
  var a = 4;
  while (a < 9) {
    a = a < 4 ? a + 1 :
        a > 4 ? a - 1 :
        rand(10);
    update(a)
    delay(1000)
  }
}

function update(a) {
  // TODO: Update display
}
```

Translates to the following s-expression in the visual editor.

```ujack
(def (main)
  (def a 4)
  (while (< a 9)
    (set a (? (< a 4) (+ a 1)
           (? (> a 4) (- a 1)
           (rand 10))))
    (update a)
    (delay 1000)
  )
)

(def (update a))
```

Or the following text format:

```
main() {
  let a = 4
  while a < 9 {
    a = a < 4 ? a + 1 :
        a > 4 ? a - 1 :
        rand(10)
    update(a)
    delay(1000)
  }
}

update(a) { }
```

Which compiles to the following assembly:

```jack-asm
:0
  VARS 1
  SET 0 4
  WHILE LT GET 0 9
    SET 0
      CHOOSE LT GET 0 4
        ADD GET 0 1
        CHOOSE GT GET 0 4
          SUB GET 0 1
          RAND 10
    RUN :1
    DELAY 1000
  LOOP
  FREE 1
  DONE

:1
  DONE
```

--------

16 named ports (external functions)


79 opcodes

32 GET opcodes?
