#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>


enum opcode {
  // ISLT,  // var var: Skip one if A < D
  // ISLE,  // var var: Skip one if A <= D
  // ISGE,  // var var: Skip one if A >= D
  // ISGT,  // var var: Skip one if A >  D
  // ISEQ,  // var var: Skip one if A =  D
  // ISNE,  // var var: Skip one if A != D
  // ISLTN, // var lits: Skip one if A < D
  // ISLEN, // var lits: Skip one if A <= D
  // ISGEN, // var lits: Skip one if A >= D
  // ISGTN, // var lits: Skip one if A >  D
  ISEQN, // var lits: Skip one if A =  D
  ISNEN, // var lits: Skip one if A != D
  JMP,   // rbase jump: Set base to A and jump relative D.
  MOV, // dst var: Copy D to A
  NOT, // dst var: Set A to boolean not of D
  UNM, // dst var: Set A to -D (unary minus)
  ABS, // dst var: Set A to |D| (absolute value)
  ADDVN, // dst var lits: A = B + C
  SUBVN, // dst var lits: A = B - C
  MULVN, // dst var lits: A = B * C
  DIVVN, // dst var lits: A = B / C
  MODVN, // dst var lits: A = B % C
  ADDNV, // dst var lits: A = C + B
  SUBNV, // dst var lits: A = C - B
  MULNV, // dst var lits: A = C * B
  DIVNV, // dst var lits: A = C / B
  MODNV, // dst var lits: A = C % B
  ADDVV, // dst var var: A = B + C
  SUBVV, // dst var var: A = B - C
  MULVV, // dst var var: A = B * C
  DIVVV, // dst var var: A = B / C
  MODVV, // dst var var: A = B % C
  KNUM,  // dst lits Set A integer constant D
  KLIT,  // dst lits Set A to 16 bit signed integer D
};

// ┌─┬─┬─┬──┐
// │B│C│A│OP│
// ├─┴─┼─┼──┤
// │ D │A│OP│
// └───┴─┴──┘

#define A(word) ((word) >> 8 & 0xff)
#define B(word) ((word) >> 24 & 0xff)
#define C(word) ((word) >> 16 & 0xff)
#define D(word) ((word) >> 16 & 0xffff)
#define OP(word) (enum opcode)((word) & 0xff)

#define ABC(op, a, b, c) ((uint8_t)(b) << 24 | (uint8_t)(c) << 16 | (uint8_t)(a) << 8 | (enum opcode)(op))
#define AD(op, a, d) ((uint16_t)(d) << 16 | (uint8_t)(a) << 8 | (enum opcode)(op))

#define UNARY(op) slots[A(code)] = op slots[D(code)]
#define BINARYVN(op) slots[A(code)] = slots[B(code)] op (int8_t)C(code)
#define BINARYNV(op) slots[A(code)] = (int8_t)C(code) op slots[B(code)]
#define BINARYVV(op) slots[A(code)] = slots[B(code)] op slots[C(code)]

const char* eval(int64_t* slots, int num, uint32_t* codes, int len, int64_t* nums) {
  int i;
  // int count = 0;
  for (i = 0; i < len; i++) {
    uint32_t code = codes[i];
    // count = (count + 1) % 1000000;
    // if (count == 0) {
    //   printf("\n");
    //   int j;
    //   for (j = 0; j < num; j++) {
    //     printf("%d: %ld\n", j, slots[j]);
    //   }
    // }
    // if (count < 4) printf("i=%d bc=%08x op=%d a=%d b=%d c=%d d=%d\n", i, code, OP(code), A(code), B(code), C(code), D(code));
    switch (OP(code)) {
      case ISEQN: if (slots[A(code)] == (int16_t)D(code)) i++; continue;
      case ISNEN: if (slots[A(code)] != (int16_t)D(code)) i++; continue;
      case JMP: i += (int16_t)D(code); continue;
      case MOV: UNARY(); continue;
      case UNM: UNARY(-); continue;
      case NOT: UNARY(!); continue;
      case ABS: slots[A(code)] = slots[D(code)] < 0 ? -slots[D(code)] : slots[D(code)]; continue;
      case ADDVN: BINARYVN(+); continue;
      case SUBVN: BINARYVN(-); continue;
      case MULVN: BINARYVN(*); continue;
      case DIVVN: BINARYVN(/); continue;
      case MODVN: BINARYVN(%); continue;
      case ADDNV: BINARYNV(+); continue;
      case SUBNV: BINARYNV(-); continue;
      case MULNV: BINARYNV(*); continue;
      case DIVNV: BINARYNV(/); continue;
      case MODNV: BINARYNV(%); continue;
      case ADDVV: BINARYVV(+); continue;
      case SUBVV: BINARYVV(-); continue;
      case MULVV: BINARYVV(*); continue;
      case DIVVV: BINARYVV(/); continue;
      case MODVV: BINARYVV(%); continue;
      case KNUM: slots[A(code)] = nums[D(code)]; continue;
      case KLIT: slots[A(code)] = (int16_t)D(code); continue;
    }
  }
  for (i = 0; i < num; i++) {
    printf("%d: %"PRId64"\n", i, slots[i]);
  }
  return NULL;
}

int main() {
  int64_t slots[] = {0,0,0};
  uint32_t code[] = {
    AD(KNUM, 0, 0), // 1000000000
    AD(KLIT, 1, 0),
    ABC(ADDVV, 1, 1, 0),
    ABC(SUBVN, 0, 0, 1),
    AD(ISEQN, 0, 0),
    AD(JMP, 0, -4),
  };
  int64_t nums[] = { 10000000 };
  const char* err = eval(slots, 2, code, 6, nums);
  if (err) {
    fprintf(stderr, "Error: %s\n", err);
    return -1;
  }
  return 0;
}
