#include "utils.h"

value_t Map(state_t* S) {
  return RawPair(S, MAP, Bool(false), Bool(false));
}

static bool recursiveSet(state_t* S, value_t map, value_t key, value_t value, value_t slot, int32_t bits) {
  pair_t node = getPair(S, map);

  // If the key matches, set the value.
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) {
      // Only return true if the value changed
      if (!eq(mapping.right, value)) {
        setPair(S, node.left, key, value);
        return true;
      }
      return false;
    }
  }

  // If we find a free slot and havn't seen one yet, record it.
  if (falsy(node.left) && slot.type == BOOLEAN) {
    slot = map;
  }

  // If there are no further subtrees, stop here.
  if (node.right.type != PAIR) {
    // If we had seen an empty slot, use it.
    if (slot.type != BOOLEAN) {
      setLeft(S, slot, Pair(S, key, value));
      return true;
    }
    // Otherwise, create a new split and populate one side with slot and value.
    slot = Pair(S, Pair(S, key, value), Bool(false));
    if (bits & 1) {
      setPair(S, map, node.left, Pair(S, slot, Bool(false)));
    }
    else {
      setPair(S, map, node.left, Pair(S, Bool(false), slot));
    }
    return true;
  }

  // If there is a subtree on the matching side, recurse into it.
  pair_t split = getPair(S, node.right);
  value_t side = bits & 1 ? split.left : split.right;
  if (side.type == PAIR) {
    return recursiveSet(S, side, key, value, slot, bits >> 1);
  }

  // If we've reached this end and had seen a slot, use it now.
  if (slot.type != BOOLEAN) {
    setLeft(S, slot, Pair(S, key, value));
    return true;
  }

  // Otherwise fill out the other half of the split.
  side = Pair(S, Pair(S, key, value), Bool(false));
  if (bits & 1) {
    setPair(S, node.right, side, split.right);
  }
  else {
    setPair(S, node.right, split.left, side);
  }
  return true;
}

value_t mapSet(state_t* S, value_t map, value_t key, value_t value) {
  if (map.type != MAP) return Bool(false);
  return Bool(recursiveSet(S, map, key, value, Bool(false), hash(key)));
}

static bool recursiveDelete(state_t* S, value_t map, value_t key, int32_t bits) {
  pair_t node = getPair(S, map);
  // If we find the key, remove the mapping and we're done!
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) {
      setPair(S, map,
        Bool(false),
        node.right);
      return true;
    }
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return false;
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  return side.type == PAIR && recursiveDelete(S, side, key, bits >> 1);
}

value_t mapDelete(state_t* S, value_t map, value_t key) {
  if (map.type != MAP) return Bool(false);
  return Bool(recursiveDelete(S, map, key, hash(key)));
}

static value_t recursiveRead(state_t* S, value_t map, value_t key, int32_t bits) {
  pair_t node = getPair(S, map);
  // If we find the key, return the value!
  if (node.left.type == PAIR) {
    pair_t mapping = getPair(S, node.left);
    if (eq(mapping.left, key)) return mapping.right;
  }
  // If there is no tree/split yet, stop looking, it's not here.
  if (node.right.type != PAIR) return Bool(false);
  // Otherwise, Look down the split.
  pair_t split = getPair(S, node.right);
  // If the branch already exists, recurse down it.
  value_t side = (bits & 1) ? split.left : split.right;
  if (side.type == PAIR) return recursiveRead(S, side, key, bits >> 1);
  return Bool(false);
}

value_t mapGet(state_t* S, value_t map, value_t key) {
  if (map.type != MAP) return Bool(false);
  return recursiveRead(S, map, key, hash(key));
}
