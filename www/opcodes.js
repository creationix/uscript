// Mapping from builtin opcode to arity
(function (opcodes) {
  try { module.exports = opcodes; }
  catch (err) { window.opcodes = opcodes; }
})({
  do: undefined, end: undefined, // do ... end
  printnum: 1, // (num)
  aprint: 1, // (ptr)
  buffer: undefined, // length prefixed bytes
  mode: 2, // pinMode(pin, mode)
  read: 1, // digitalRead(pin)
  write: 2, // digitalWrite(pin, value)
  aread: 1, // analogRead(pin)
  pwrite: 2, // analogWrite(pin, value)
  ibegin: 2, // Wiring.begin(sda, scl)
  ifrom: 3, // Wiring.requestFrom(address, quantity, stop)
  istart: 1, // Wiring.beginTransmission(address)
  istop: 1, // Wiring.endTransmission(stop)
  iwrite: 1, // Wiring.write(byte)
  iavailable: 0, // Wiring.available()
  iread: 0, // Wiring.read()
  delay: 1, // delay(ms)
  func: 1, // (body)
  call: 2, // (shift, ptr)
  alloc: 1, // (size)
  aget: 2, // (ptr, offset)
  aset: 3, // (ptr, offset, value)
  alen: 1, // (ptr)
  free: 1, // (ptr)
  gget: 1, // (index)
  gset: 2, // (index, value)
  get: 1, // (index)
  set: 2, // (index, value)
  incr: 1, decr: 1, // (index)
  incrmod: 2, decrmod: 2, // (index, mod)
  forever: 1, // (action)
  while: 2, // (condition, action)
  wait: 1, // (condition)
  if: 2, // (condition, action)
  elseif: 2, // (condition, action)
  else: 1, // (action)
  add: 2, sub: 2, mul: 2, div: 2, mod: 2, neg: 1,
  band: 2, bor: 2, bxor: 2, bnot: 1, lshift: 1, rshift: 2,
  and: 2, or: 2, xor: 2, not: 1, choose: 3,
  gt: 2, gte: 2, lt: 2, lte: 2, eq: 2, neq: 2,
  srand: 1, // deadbeef_srand(seed)
  rand: 1, // deadbeef_rand(modulus)
  restart: 0, chipid: 0, flashchipid: 0, cyclecount: 0, getfree: 0,

  // Function  definition
  def: undefined,
  // Variables
  const: 2, var: 2,
});
