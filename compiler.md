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

------------------------------


**WARNING** What follows is out of date and may be incorrect, I'll finish this
soon, but it's family time!  The plan is to show a few sample programs using
raw AST and various textual formats.  We'll then go into how the compiler will
work and what optimizations it will make.

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


# Misc.

16 named ports (external functions)

79 opcodes

32 GET opcodes?
