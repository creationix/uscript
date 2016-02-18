#include "utils.h"
#include <stdlib.h>

value_t Char(int32_t code) {
  return (value_t){
    .gc = true,
    .type = CHARACTER,
    .value = code
  };
}

value_t Bool(bool value) {
  return (value_t){
    .gc = true,
    .type = BOOLEAN,
    .value = value
  };
}

value_t Int(int32_t value) {
  return (value_t){
    .gc = true,
    .type = INTEGER,
    .value = value & 0x7ffffff
  };
}

value_t Integer(state_t* S, int64_t value) {
  // If the value fits within the int27 range, store unboxed.
  if (value >= -0x4000000 && value < 0x4000000) {
    return Int((int32_t)value);
  }

  // Otherwise it must be stored in an int64 field.
  int32_t i = 0;
  while (true) {
    int64_t slot = S->ints[i];
    if (!slot) {
      S->ints[i] = value;
      break;
    }
    if (slot == value) break;
    i++;
    if (i < S->num_ints) {
      continue;
    }
    int32_t j = i + START_INTS;
    S->num_ints = j;
    S->ints = realloc(S->ints,  sizeof(int64_t) * (size_t)j);
    while (j-- > i) {
      S->ints[j] = 0;
    }
  }
  return (value_t){
    .gc = true,
    .type = BOX_INTEGER,
    .value = i
  };
}


static int gcd(int64_t a, int64_t b) {
  int64_t c;
  while (a != 0) {
    c = a;
    a = b % a;
    b = c;
  }
  return b;
}


int64_t toInt(state_t* S, value_t value) {
  switch (value.type) {
    case INTEGER: return value.value;
    case BOX_INTEGER: return S->ints[value.value];
    case RATIONAL: {
      pair_t pair = getPair(S, value);
      return toInt(S, pair.left) / toInt(S, pair.right);
    }
    default: return 0; // TODO: should this thow an error?
  }
}

value_t Rational(state_t* S, int64_t num , int64_t dem) {
  if (dem == 0) {
    num = num > 0 ? 1 : num < 0 ? -1 : 0;
  }
  else if (num == 0) {
    dem = 1;
  }
  else if (dem != 1) {
    int g = gcd(num, dem);
    num /= g;
    dem /= g;
    if (dem < 0) {
      dem = -dem;
      num = -num;
    }
  }
  if (dem == 1) return Integer(S, num);
  return RawPair(S, RATIONAL, Integer(S, num), Integer(S, dem));
}

rational_t getRational(state_t* S, value_t value) {
  switch (value.type) {
    case INTEGER: return (rational_t){
      .num = value.value,
      .dem = 1
    };
    case BOX_INTEGER: return (rational_t){
      .num = S->ints[value.value],
      .dem = 1
    };
    case RATIONAL: {
      pair_t pair = getPair(S, value);
      return (rational_t){
        .num = toInt(S, pair.left),
        .dem = toInt(S, pair.right)
      };
    }
    default: return (rational_t){
      .num = 0,
      .dem = 1
    };
  }
}

static rational_t ratAdd(rational_t a, rational_t b) {
  // If the denominators match, we can do simple addition
  // This also handles -Inf + Inf = NaN since .num is normalized to -1 and 1.
  if (a.dem == b.dem) {
    return (rational_t){
      .num = a.num + b.num,
      .dem = a.dem
    };
  }

  // Infinity + anything is still Infinity
  if (a.dem == 0) return a;
  if (b.dem == 0) return b;

  // Use the GCD to pull out common factors while adding.
  // We divide as early as possible to prevent integer overflows.
  // Otherwise we would just defer to the Rational constructor's factoring.
  int64_t g = gcd(a.dem, b.dem);
  return (rational_t){
    .num = a.num * (b.dem / g) +
           b.num * (a.dem / g),
    .dem = (a.dem / g) * b.dem
  };
}
static rational_t ratSub(rational_t a, rational_t b) {
  return ratAdd(a, (rational_t){
    .num = -b.num,
    .dem = b.dem
  });
}
static rational_t ratMul(rational_t a, rational_t b) {
  return (rational_t){
    .num = a.num * b.num,
    .dem = a.dem * b.dem
  };
}
static rational_t ratDiv(rational_t a, rational_t b) {
  return (rational_t){
    .num = a.num * b.dem,
    .dem = a.dem * b.num
  };
}

value_t numberAdd(state_t* S, value_t left, value_t right) {
  if (left.type == INTEGER && right.type == INTEGER) {
    return Integer(S, left.value + right.value);
  }
  rational_t r = ratAdd(getRational(S, left), getRational(S, right));
  return Rational(S, r.num, r.dem);
}

value_t numberSub(state_t* S, value_t left, value_t right) {
  if (left.type == INTEGER && right.type == INTEGER) {
    return Integer(S, left.value - right.value);
  }
  rational_t r = ratSub(getRational(S, left), getRational(S, right));
  return Rational(S, r.num, r.dem);
}

value_t numberMul(state_t* S, value_t left, value_t right) {
  if (left.type == INTEGER && right.type == INTEGER) {
    return Integer(S, left.value * right.value);
  }
  rational_t r = ratMul(getRational(S, left), getRational(S, right));
  return Rational(S, r.num, r.dem);
}

value_t numberDiv(state_t* S, value_t left, value_t right) {
  if (left.type == INTEGER && right.type == INTEGER) {
    return Rational(S, left.value, right.value);
  }
  rational_t r = ratDiv(getRational(S, left), getRational(S, right));
  return Rational(S, r.num, r.dem);
}

value_t numberIDiv(state_t* S, value_t left, value_t right) {
  int64_t n = toInt(S, left);
  int64_t d = toInt(S, right);
  if (d) return Integer(S, n / d);
  return Rational(S, 1, 0);
}

value_t numberMod(state_t* S, value_t left, value_t right) {
  int64_t n = toInt(S, left);
  int64_t d = toInt(S, right);
  if (d) return Integer(S, n % d);
  return Rational(S, 1, 0);
}
