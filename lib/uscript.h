#ifndef USCRIPT_H
#define USCRIPT_H
#include <stdint.h>
#include <stddef.h>

// Override these defines to tweak to your memory/program constraints.
#ifndef SIZE_VARS
  #define SIZE_VARS 64 // fixed int of global int sized variables
#endif
#ifndef SIZE_STACK
  #define SIZE_STACK 64 // fixed max height for stack with int sized slots
#endif
#ifndef SIZE_DATA
  #define SIZE_DATA 1024 // fixed byte size of raw global memory
#endif
#ifndef SIZE_STUBS
  #define SIZE_STUBS 64 // Fixed int of pointers for dynamic subroutines.
#endif
#if defined(NUMBER_TYPE)
  typedef NUMBER_TYPE number;
#else
  typedef int number;
#endif

struct uState;

struct user_func {
  const char* name;
  // Signature for user functions
  unsigned char* (*fn)(struct uState* S, unsigned char* pc, number* res);
};

struct uState {
  void* (*malloc) (size_t size);
  void (*free) (void* ptr);
  number vars[SIZE_VARS];
  number stack[SIZE_STACK];
  int stack_top;
  unsigned char data[SIZE_DATA];
  unsigned char* stubs[SIZE_STUBS];
  struct user_func* funcs;
  int num_funcs;
};

enum opcodes {
  /* variables */
  OP_SET = 128, OP_GET, OP_INCR, OP_DECR,
  /* stack manipulation */
  OP_READ, OP_WRITE, OP_INSERT, OP_REMOVE,
  /* data space manipulation */
  OP_POKE, OP_PEEK,
  /* stubs */
  OP_DEF, OP_RM, OP_RUN,
  /* control flow */
  OP_IF, OP_ELIF, OP_ELSE, OP_MATCH, OP_WHEN, OP_WHILE, OP_DO, OP_FOR, OP_WAIT,
  /* logic (short circuit and/or) */
  OP_NOT, OP_AND, OP_OR, OP_XOR,
  /* bitwise logic */
  OP_BNOT, OP_BAND, OP_BOR, OP_BXOR, OP_LSHIFT, OP_RSHIFT,
  /* comparison */
  OP_EQ, OP_NEQ, OP_GTE, OP_LTE, OP_GT, OP_LT,
  /* math */
  OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_ABS,
  /* random */
  OP_RAND, OP_SRAND,
};
#define OP_USER_START (OP_SRAND + 1)

const char* op_to_name(struct uState* S, int op);
int name_to_op(struct uState* S, const char* name, int len);
int compile(struct uState* S, unsigned char* program);
unsigned char* skip(struct uState* S, unsigned char* pc);
unsigned char* eval(struct uState* S, unsigned char* pc, number* res);

#endif
