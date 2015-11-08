# Jack to uScript Compiler

Programs are written in Jack language and then compiled down to uScript
bytecode.

In this higher-level language we have a optional textual syntax as well as
conveniences like symbols, variables, labels, etc.

Programs can be authored in text format or via graphical means and manipulate
the AST directly.  The AST (Abstract Syntax Tree using s-expressions) and the text formats are interchangeable.

Since I enjoy writing parsers so much, there will probably be several text
formats that vary from lisp style to C-like to python like.

The AST stores more information than the raw bytecode (like comments, function
names, symbol names, and variable names) and has it's own binary format.  You
can disassemble the bytecode back to AST, but it will be somewhat obfuscated.

# Jack Language Semantics

The core semantics of Jack follow closely to the opcodes in the uscript VM.  However there are a few features added on top that are easy to compile out and
make authoring code much nicer.

## Namespaces

Jack is a namespaced language.  Code lives in modules that are represented by
filesystem-like trees.  For example, suppose there is a library for talking to
H616K33 ICs (like what powers AdaFruit's LED backpacks) using uscript's I2C
primitives.

Suppose the following tree:

```
creationix/
└── ht16k33/
    ├── alphafonttable.jack
    ├── index.jack
    └── numbertable.jack
```

All exported symbols in the `index.jack` file will be prefixed with
`creationix.ht16k33`, the buffer exported by `numbertable.jack` will have the
identifier `creationix.ht16k33.numbertable`.

But from inside `index.jack` code lives inside the `creationix.ht16k33`
namespace and so can reference the number table as just `numbertable`.  Using
this direct filepath to namespace mapping, there is never a need to import
files, just use the symbols you need and the compiler will know where to look.

### Import to create short aliases

You can create aliases using the `import` keyword and this is recommended for
using third-party libraries to strip out the author portion.  So for example, a
file using my library will have:

```jack
-- Import a deep symbol
import creationix.ht16k33

bus = isetup(2, 3)
const addr = 0x71

-- Use the shortened symbol
ht16k33.begin(bus, addr)
```

### Nested in-file Namespaces

Sometimes you want to create a sub-namespace but not create a sub-folder with
index file or multiple individual files.  This can be done with the `namespace`
keyword.

```jack
-- Currently I'm in the creationix.sample namespace.

export name = "Tim Caswell"
export age = 33

namespace stats {
  -- Everything in here is prefixed with creationix.sample.stats
  export handwriting = 1
  export typing = 5
  export programming = 9
  export painting = 3
}
```

Also you can simply use dot notation.

```jack
-- Still in the creationix.sample namespace.
-- will be exported as creationix.sample.stats.more
export stats.more = true
```

### Symbol Search Path

The search path is simple, the compiler starts looking in your current namespace
and then looks up all the way to the root.

So if I'm in `creationix.sample`, have an `import makrboy.blinky` and ask for `blinky.go` it will search in the following order.

- `creationix.sample.makrboy.blinky.go`
- `creationix.makrboy.blinky.go`
- `makrboy.blinky.go`

Every symbol will search all possible files that may contain the symbol.  The
symbol `makrboy.blinky.go` could be found in any of the following.

- `makrboy.jack`
- `makrboy/index.jack`
- `makrboy/blinky.jack`
- `makrboy/blinky/index.jack`
- `makrboy/blinky/go.jack`
- `makrboy/blinky/go/index.jack`

It won't stop at the first match found so order doesn't matter here.  The
compiler will look in all possible places to ensure the symbol is unique.

Later when more file extensions are added (more syntaxes and formats) all
permutations will be checked as well.  Perhaps we add a lisp-stype format called
`.jackl` or a binary representation of the AST for graphical programming called
`.jast`.

### No conflicts allowed

It is illegal to have any conflicting symbols.  There are many ways to create
conflicts.

- The same symbol twice in the same file.
- Importing multiple paths ending in the same name.  For example both
  `makrboy.blinky` and `creationix.blinky`.
- Conflicts between imports and local symbols.
- Conflicts between symbols in the index file and named files in the same or
  parent trees..
- Conflicts between two files with the same path, but different extensions.

The compiler can easily find such conflicts and will happily let you know there
is a problem.

### Public and Private Symbols

Very often in hardware programming you want to label some constant numbers (such
as GPIO pin numbers or special I2C addresses).  The VM works best if it's given
plain numbers in these cases, but your code will be 100x more readable if these
magic numbers are given names.

Jack has a very simple and strict two-level scope.  Functions can only exist at
the top level, there are no closures or lambdas in this language.  Variables
exist inside functions and constants live outside them. The function itself is a
constant.

All constants are private by default, but can be made public (available outside
the current file) by prefixing with the `export` keyword.

```jack
-- This is a private constant.  Any time the identifier `apple` is seen in this
-- file, the compiler will literally replace it with `10`.
apples = 10

-- This is an exported constant.  If the current namespace is `market` then
-- there will be a global constant of `market.fruit` that anyone can use.
export fruit = "apple"

-- `slice` here is a private constant referencing this function.  Only code
-- in this file can access it.
-- `thing` is a local variable inside the function.
slice(thing) {
  -- `count` is another local variable.
  count = 10
}

-- Here `cook` is a public symbol and anyone knowing it's namespace can call it.
export cook() {
  slice(fruit, apples)
}
```

### Whole file Exports

Sometimes you want to export a large value such as a font as part of your
library, but you want to organize this into it's own file in your source code.

This can be done with whole-file exports.  Every file is allowed one `export`
that does not contain a name.

```jack
-- This file is `dadams/guide/question/answer.jack`
export 42
```

Functions can be exported this way too.

```jack
export (a, b, c) {
  -- ...
}
```

It's convention to put a space after the export keyword so it doesn't look like
a local function named `export()`. TODO: Think of something better, I don't like
the function export syntax looking like a private function.

It's perfectly to have other code in the same file, the exported function might
use some private constants or functions, for example.

## Literals

There are a few types of literals allowed for values in constants and local
variables.

```jack
-- Boolean
good = true
-- Decimal integer
number = 100
-- Binary integer
bits = 0b100101010101
-- Hexadecimal integer
magic = 0xdeadbef
-- String buffer
name = "Jack"
-- Byte array buffer
fibb = '0x01 0x12 0x35 0x8d'
```

Also data structures using pairs can be literals with any expression as values.

```jack
-- A list from 1 to 5
count = [1 2 3 4 5]
-- A map from x to x squared
squares = [1:1 2:4 3:9 4:16]
-- Raw cons pair
pair = (true,false)
```

TODO: find better syntax for raw cons pair, it looks too much like function's
parameter head.

Functions are not first-class values and cannot be stored in data structures.

For constant and export literals, only other constants can be used in the
sub-expressions.  These are evaluated at compile time.

If the final result requires boxing (large integers, buffers, and data
structures), the value will be included in the data portion of the code to flash
and references inline in all places the symbol appears.  Thus all references
globally to a buffer will share the exact same mutable buffer.
