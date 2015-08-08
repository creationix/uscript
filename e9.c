#define MAX_VALUES 100

typedef enum {
  // Numbers below 128 represent themselves as literal numbers.
  ADD = 128, SUB, MUL, DIV, MOD, NEG,
  GT, GTE, LT, LTE, EQ, NEQ,
  AND, OR, XOR, NOT,
  INT32, INT64,
  DR, // pin - digital read gpio
  AR, // pin - analog read gpio
  SR, // space - port
  W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14, W15,
  DW,         // pin value
  AW,         // pin value
  PM,         // pin mode
  SW,         // addr/space value
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
  B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15,
  BAND, BOR, BXOR, BNOT, LSHIFT, RSHIFT,
  CALL,  // start, address
  CCALL, // start, address
} expr;

typedef enum {
  ISTC, ISFC, // target/source jump
  IST, ISF,   // expr jump
  JMP,        // jump
  COPY,       // target/source
  RADD, RSUB, RISUB, RMUL, RDIV, RIDIV, RMOD, RIMOD, // target/source
  ADDI, SUBI, ISUBI, MULI, DIVI, IDIVI, MODI, IMODI, // target/literal
  RNEG,       // target
} opcode;

typedef long int number;

number vStack[MAX_VALUES];



AND 1 2

if condition yes no


iJump condition target
Jump target
