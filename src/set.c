#include "utils.h"

value_t Set(state_t* S) {
  return RawPair(S, SET, Bool(false), Bool(false));
}

// setNode = (value, split)
// setNode = (value, (false, rightNode))
// setNode = (value, (leftNode, false))
// setNode = (value, (leftNode, rightNode))
// An false value means the slot is empty (it might have held a deleted value)
// An false split means both sides are unset
// The split may have one side false and the other set.
//
// recursiveAdd needs to search for a matching value.
// If the value is not found, then the first empty slot that was encountered
// is used to add the value.  If there was none, add a new slot (and possibly
// a new split to hold it)
static bool recursiveAdd(state_t* S, value_t set, value_t value, value_t slot, int32_t bits) {
  pair_t node = getPair(S, set);
  // If the value is already here, abort.
  if (eq(node.left, value)) return false;

  // If we find a free slot and havn't seen one yet, record it.
  if (falsy(node.left) && slot.type == BOOLEAN) {
    slot = set;
  }

  // If there are no further subtrees, stop here.
  if (node.right.type != PAIR) {
    // If we had seen an empty slot, use it.
    if (slot.type != BOOLEAN) {
      setLeft(S, slot, value);
      return true;
    }
    // Otherwise, create a new split and populate one side with slot and value.
    slot = Pair(S, value, Bool(false));
    if (bits & 1) {
      setPair(S, set, node.left, Pair(S, slot, Bool(false)));
    }
    else {
      setPair(S, set, node.left, Pair(S, Bool(false), slot));
    }
    return true;
  }

  // If there is a subtree on the matching side, recurse into it.
  pair_t split = getPair(S, node.right);
  value_t side = bits & 1 ? split.left : split.right;
  if (side.type == PAIR) {
    return recursiveAdd(S, side, value, slot, bits >> 1);
  }

  // If we've reached this end and had seen a slot, use it now.
  if (slot.type != BOOLEAN) {
    setLeft(S, slot, value);
    return true;
  }

  // Otherwise fill out the other half of the split.
  side = Pair(S, value, Bool(false));
  if (bits & 1) {
    setPair(S, node.right, side, split.right);
  }
  else {
    setPair(S, node.right, split.left, side);
  }
  return true;
}

value_t setAdd(state_t* S, value_t set, value_t value) {
  if (set.type != SET) return Bool(false);
  return Bool(recursiveAdd(S, set, value, Bool(false), hash(value)));
}

static bool recursiveHas(state_t* S, value_t set, value_t value, int32_t bits) {
  pair_t node = getPair(S, set);
  // If we find the value, we're done!
  if (eq(node.left, value)) return true;
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveHas(S, side, value, bits >> 1);
}

value_t setHas(state_t* S, value_t set, value_t value) {
  if (set.type != SET) return Bool(false);
  return Bool(recursiveHas(S, set, value, hash(value)));
}

static bool recursiveRemove(state_t* S, value_t set, value_t value, int32_t bits) {
  pair_t node = getPair(S, set);
  // If we find the value, remove it and we're done!
  if (eq(node.left, value)) {
    setPair(S, set,
      Bool(false),
      node.right);
    return true;
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveRemove(S, side, value, bits >> 1);
}

value_t setRemove(state_t* S, value_t set, value_t value) {
  if (set.type != SET) return Bool(false);
  return Bool(recursiveRemove(S, set, value, hash(value)));
}
