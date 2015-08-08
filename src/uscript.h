#ifndef USCRIPT_H
#define USCRIPT_H
// Feel free to override these to fit your constraints / needs.

// This is the c-type of the basic number type. Use some type of signed integer.
#ifndef NUMBER_TYPE
#define NUMBER_TYPE int
#endif

// Make sure to leave enough heap for bytecode that is handled externally.

typedef NUMBER_TYPE integer;

enum opcode {
  // Consume one expression and write to register slot
  W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14, W15,
  // Set program labels
  L0, L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, L13, L14, L15,
  // Jump to program labels
  J0, J1, J2, J3, J4, J5, J6, J7, J8, J9, J10, J11, J12, J13, J14, J15,
  // Conditional Jumps, accept two 4-bit arguments for register and labels
  IS, ISN,
  // Statement math
  RADD, RSUB, RISUB, RMUL, RDIV, RIDIV, RMOD, RIMOD, RNEG, INCR, DECR,
  // function bodies
  CALL, DEF, END,
  // Digital Write, Analog Write (port, value)
  DW, AW, PM,
  // Delay(ms)
  DELAY, UDELAY,
  // print(val)
  PRINT, PRINTS,
  // // Socket write (address, port, value)
  // SW,
};

// Expression tree nodes
enum node {
  // 0mxxxxxx mxxxxxxx: variable length unsigned integer literal.
  // Read register slots 0 - 15
  R0 = 128, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
  // Basic math
  ADD, SUB, MUL, DIV, MOD, NEG,
  // Basic comparison
  EQ, NEQ, GT, LT, GTE, LTE,
  // Basic logic
  AND, OR, XOR, NOT,
  // Conditionals
  IF, ELIF, ELSE,
  // GPIO reads (digital / analog)
  DR, AR,
  // // Read from virtual slots (network ports)
  // SR,
};

void uscript_setup();
integer* uscript_run(unsigned char* code);

#endif
