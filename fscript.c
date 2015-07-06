#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef int32_t value;

typedef struct{
  size_t top;
  size_t len;
  value data[];
} *Stack;

typedef const char* status;

typedef status(*word)(Stack stack, value data);

typedef struct {
  word fn;
  value val;
} code;

// Drop the top item in the stack
static status op_drop(Stack stack, value data) {
  if (!stack->top) return "Can't drop from empty stack";
  stack->top--;
  return NULL;
}

// Swap the top two items in the stack
static status op_swap(Stack stack, value data) {
  if (stack->top < 2) return "Need at least two items to swap in stack";
  value temp = stack->data[stack->top - 1];
  stack->data[stack->top - 1] = stack->data[stack->top - 2];
  stack->data[stack->top - 2] = temp;
  return NULL;
}

// Duplicate the top item in the stack.
static status op_dup(Stack stack, value data) {
  if (stack->top >= stack->len) return "Stack full, can't dup.";
  stack->data[stack->top] = stack->data[stack->top - 1];
  stack->top++;
  return NULL;
}

static status op_remove(Stack stack, value data) {
  return "TODO: Implement remove";
}

static status op_insert(Stack stack, value data) {
  return "TODO: Implement insert";
}

static status op_copy(Stack stack, value data) {
  return "TODO: Implement copy";
}

static status op_number(Stack stack, value data) {
  if (stack->top >= stack->len) return "Stack full, can't add number";
  stack->data[stack->top++] = data;
  return NULL;
}

static const word builtins[] = {
  op_drop, op_swap, op_dup, op_remove, op_insert, op_copy,
  op_number,
};

static const char op_names[] =
  "drop\0swap\0dup\0remove\0insert\0copy\0"
  "number\0"
  "\0";

word name_to_word(const char* name) {
  const word* fn = builtins;
  const char* op = op_names;
  while (*op) {
    if (strcmp(name, op) == 0) return *fn;
    fn++;
    while (*op++);
  }
  return 0;
}

const char* word_to_name(word x) {
  const word* fn = builtins;
  const char* op = op_names;
  while (*op) {
    if (*fn == x) return op;
    fn++;
    while (*op++);
  }
  return 0;
}

int main() {
  code sample[] = {
    {op_number, 1}, {op_number, 2}, {op_swap, 0}, {NULL},
  };
  code* pc = sample;
  Stack stack = malloc(sizeof(*stack) + sizeof(value)*10);
  stack->top = 0;
  stack->len = 10;
  while (pc->fn) {
    status res = pc->fn(stack, pc->val);
    if (res) {
      fprintf(stdout, "Error: %s\n", res);
      exit(-1);
    }
    pc++;
  }
  for (int i = 0; i < stack->top; i++) {
    printf("%d - %d\n", i, stack->data[i]);
  }

  return 0;
}
