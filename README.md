## Integers

Integers in source push themselves on the stack.

## Basic operators

The basic operators pop their needed arguments and push their result.

- Arithematic: `+`, `-`, `*`, `/`, `%`
- Comparison: `<` `>` `<=` `>=` `=` `<>`
- Logic: `and` `or` `xor` `not`
- Bitwise: `|` `&` `^` `<<` `>>` `~` (Maybe not in first version, keep it simple)

## Stack operations

- Dup (a -- a a)
- Pop (a -- )
- Over (a b -- a b a)
- ... (Add as needed)

## variables

The language uscript has 16 registers `a`-`p`.  Any time a variable is used it's
value is retrieved and pushed on the stack.

To set a variable, simply use the `set x` form where x is the variable name.

Example:

```uscript
42 set a
a 42 =
```

## a if b else c then

`if` will pop a condition off the stack.  If it's true, it will execute the
following code stopping at `else` or `then`, whichever comes first.  If it was
an `else`, it will then skip to `then`. If the condition was false, it will skip
to `else` or `then`.

Example:

```uscript
a 4 > if
  a 1 -
else
  a 2 < if
   a 1 +
  else
   3
  then
then set-a
```

## loop

1 10 1 FOR-x ... END

limit start DO .. LOOP
DO .. +LOOP
LEAVE
BEGIN .. UNTIL
BEGIN .. WHILE .. REPEAT
