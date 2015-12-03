// Mapping from builtin opcode to arity
(function (opcodes) {
  try { module.exports = opcodes; }
  catch (err) { window.opcodes = opcodes; }
})({
  // Integer operations
  NEG: 1, ADD: 2, SUB: 2, MUL: 2, DIV: 2, MOD: 2,
  // Bitwise operations
  BNOT: 1, BXOR: 2, BAND: 2, BOR: 2, LSHIFT: 2, RSHIFT: 2,
  // Comparison operations
  GT: 2, GTE: 2, LT: 2, LTE: 2, EQ: 2, NEQ: 2,
  // Logical operations
  NOT: 1, AND: 2, OR: 2, XOR: 2, CHOOSE: 3,
  // Pseudo random number generator
  SRAND: 1, RAND: 1,
  // Garbage Collection
  GC: 1,
  // Buffer operations
  LEN: 1, PEEK: 2, POKE: 3, FILL: 4, COPY: 5, INDEX: 2, FIND: 2,
  // Pair operations
  HEAD: 1, TAIL: 1, SETHEAD: 2, SETTAIL: 2,
  // List operations
  LLEN: 1, LIDX: 2, LGET: 2, LSET: 3, LDEL: 2,
  // Set operations
  SHAS: 2, SDEL: 2, SADD: 2,
  // Map operations
  MSET: 3, MGET: 2, MDEL: 2,
  // GPIO operations
  MODE: 2, WRITE: 2, READ: 1, PWRITE: 2, AREAD: 1,
  // I2C operations2
  ISETUP: 3, ISTART: 1, ISTOP: 1, IADDR: 3, IWRITE: 2, IREAD: 1,
  // Neopixel (ws2812)
  NEOPIX: 2,
  // Local variables
  LET: 2, SET: 2, GET: 1,
  // Pausing statements
  DELAY: 1, UDELAY: 1, WAIT: 1, YIELD: 0,
  // Control Flow
  IF: 2,
  // Loops / Iterators
  WHILE: 2, DOWHILE: 2, FOR: 5, EACH: 3,
  // Loop control flow
  BREAK: 0, CONTINUE: 0,
  // Function calling and definition
  FUN: undefined, RETURN: 1,
  // block
  DO: undefined, END: 0,
  // Constants
  DEF: 2,
  // Terminal I/O
  READLINE: 0, WRITELINE: 1, PRINT: 1, GETCHAR: 0, PUTCHAR: 1,
});
