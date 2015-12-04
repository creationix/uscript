// Mapping from builtin opcode to arity
(function (opcodes) {
  try { module.exports = opcodes; }
  catch (err) { window.opcodes = opcodes; }
})({
  // Integer operations
  neg: 1, add: 2, sub: 2, mul: 2, div: 2, mod: 2,
  // Bitwise operations
  bnot: 1, bxor: 2, band: 2, bor: 2, lshift: 2, rshift: 2,
  // Comparison operations
  gt: 2, gte: 2, lt: 2, lte: 2, eq: 2, neq: 2,
  // Logical operations
  not: 1, and: 2, or: 2, xor: 2, choose: 3,
  // Pseudo random number generator
  srand: 1, rand: 1,
  // Garbage Collection
  gc: 1,
  // Buffer operations
  len: 1, peek: 2, poke: 3, fill: 4, copy: 5, index: 2, find: 2,
  // Pair operations
  head: 1, tail: 1, sethead: 2, settail: 2,
  // List operations
  llen: 1, lidx: 2, lget: 2, lset: 3, ldel: 2,
  // Set operations
  shas: 2, sdel: 2, sadd: 2,
  // Map operations
  mset: 3, mget: 2, mdel: 2,
  // GPIO operations
  mode: 2,       // pinMode(pin, mode)
  write: 2,      // digitalWrite(pin, value)
  read: 1,       // digitalRead(pin)
  pwrite: 2,     // analogWrite(pin, value)
  aread: 1,      // analogRead(pin)
  // I2C operations
  ibegin: 2,     // Wiring.begin(sda, scl)
  ifrom: 3,      // Wiring.requestFrom(address, quantity, stop)
  istart: 1,     // Wiring.beginTransmission(address)
  istop: 1,      // Wiring.endTransmission(stop)
  iwrite: 1,     // Wiring.write(byte or buffer)
  iavailable: 0, // Wiring.available()
  iread: 0,      // Wiring.read()
  // Neopixel (ws2812)
  neopix: 2,
  // Local variables
  let: 2, set: 2, get: 1,
  // Pausing statements
  wait: 1, yield: 0,
  // Control Flow
  if: 2,
  // Loops / Iterators
  forever: 1, while: 2, dowhile: 2, for: 5, each: 3,
  // Loop control flow
  break: 0, continue: 0,
  // Function calling and definition
  fun: undefined, return: 1,
  // block
  do: undefined, end: 0,
  // Constants
  def: 2,
  // Misc
  reset: 0, free: 0, id: 0,
  // Timing
  millis: 0, micros: 0, delay: 1, udelay: 1,
  // Terminal I/O
  readline: 0, writeline: 1, print: 1, getchar: 0, putchar: 1,
});
