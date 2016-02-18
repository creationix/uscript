#include "utils.h"

value_t Stack(state_t* S) {
  return RawPair(S, STACK, Int(0), Bool(false));
}

value_t stackPush(state_t* S, value_t stack, value_t value) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  value_t count = Integer(S, toInt(S, meta.left) + 1);
  setPair(S, stack,
    count,
    Pair(S, value, meta.right)
  );
  return count;
}

value_t stackPeek(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  if (meta.right.type == PAIR) {
    pair_t next = getPair(S, meta.right);
    return next.left;
  }
  return meta.right;
}

value_t stackLength(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  return meta.left;
}

value_t stackPop(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  int64_t count = toInt(S, meta.left);
  if (!count) { return Bool(false); }
  pair_t first = getPair(S, meta.right);
  setPair(S, stack,
    Integer(S, count - 1),
    first.right
  );
  return first.left;
}

value_t stackIs(state_t* S, value_t left, value_t right) {
  if (left.type != STACK || right.type != STACK) return Bool(false);
  pair_t lpair = getPair(S, left);
  pair_t rpair = getPair(S, right);
  // Make sure lengths match up.
  if (!eq(lpair.left, rpair.left)) return Bool(false);

  while (lpair.right.type == PAIR) {
    lpair = getPair(S, lpair.right);
    rpair = getPair(S, rpair.right);
    if (!eq(lpair.left, rpair.left)) return Bool(false);
  }
  return Bool(true);
}

value_t stackReverse(state_t* S, value_t stack) {
  if (stack.type != STACK) return Bool(false);
  pair_t meta = getPair(S, stack);
  value_t node = meta.right;
  value_t last = Bool(false);
  while (node.type == PAIR) {
    pair_t link = getPair(S, node);
    last = Pair(S, link.left, last);
    node = link.right;
  }
  return RawPair(S, STACK, meta.left, last);
}
