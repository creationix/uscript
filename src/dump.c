#include "dump.h"

static uint8_t* writeStr(uint8_t* start, uint8_t* end, const char* str) {
  while (start < end && *str) {
    *start++ = (uint8_t)*str++;
  }
  return start;
}

static uint8_t* writeChar(uint8_t* start, uint8_t* end, int c) {
  if (start >= end) return start;
  *start++ = (uint8_t)c;
  return start;
}

static uint8_t* writeInt(uint8_t* start, uint8_t* end, int64_t num) {
  if (start >= end) return start;
  if (num < 0) {
    *start++ = '-';
    num = -num;
  }
  else if (num == 0) {
    *start++ = '0';
    return start;
  }
  // Count the number of digits
  int64_t v = num;
  int32_t l = 0;
  do { l++; } while ((v /= 10));

  v = num;
  end = end < start + l ? end : start + l;
  // Write them out
  while (l--) {
    if (start + l < end) {
      start[l] = 0x30 + (v % 10);
    }
    v /= 10;
  }
  return end;
}

static uint8_t* writeSetNode(uint8_t* start, uint8_t* end, state_t* S, value_t node, bool* later) {
  if (falsy(node)) return start;
  pair_t pair = getPair(S, node);
  node = pair.left;
  if (truthy(node)) {
    if (*later) start = writeChar(start, end, ' ');
    start = writeValue(start, end, S, node);
    *later = true;
  }
  node = pair.right;
  if (falsy(node)) return start;
  pair_t split = getPair(S, node);
  start = writeSetNode(start, end, S, split.right, later);
  return writeSetNode(start, end, S, split.left, later);
}

static uint8_t* writeMapNode(uint8_t* start, uint8_t* end, state_t* S, value_t node, bool* later) {
  if (falsy(node)) return start;
  pair_t pair = getPair(S, node);
  if (pair.left.type == PAIR) {
    if (*later) start = writeChar(start, end, ' ');
    pair_t mapping = getPair(S, pair.left);
    start = writeValue(start, end, S, mapping.left);
    start = writeChar(start, end, ':');
    start = writeValue(start, end, S, mapping.right);
    *later = true;
  }
  node = pair.right;
  if (falsy(node)) return start;
  pair_t split = getPair(S, node);
  start = writeMapNode(start, end, S, split.right, later);
  return writeMapNode(start, end, S, split.left, later);
}

static uint8_t toHex(uint8_t nibble) {
  return nibble + (nibble < 10 ? 0x30 : 0x57);
}

// Write value to str using at most size bytes.
// Starts writing at `start` which must always be less than `end`.
// Returns new start for next code to write to.
uint8_t* writeValue(uint8_t* start, uint8_t* end, state_t* S, value_t value) {
  switch (value.type) {
    case BOOLEAN:
      return writeStr(start, end, value.value ? "true" : "false");
    case INTEGER:
    case BOX_INTEGER:
      return writeInt(start, end, toInt(S, value));
    case RATIONAL: {
      rational_t r = getRational(S, value);
      if (r.dem == 0) {
        if (r.num > 0) return writeStr(start, end, "Inf");
        if (r.num < 0) return writeStr(start, end, "-Inf");
        return writeStr(start, end, "NaN");
      }
      start = writeInt(start, end, r.num);
      start = writeChar(start, end, '/');
      return writeInt(start, end, r.dem);
    }
    case CHARACTER: {
      if (value.value < 0x20) {
        switch (value.value) {
          case 7: return writeStr(start, end, "'\\a'");
          case 8: return writeStr(start, end, "'\\b'");
          case 9: return writeStr(start, end, "'\\t'");
          case 10: return writeStr(start, end, "'\\n'");
          case 11: return writeStr(start, end, "'\\v'");
          case 12: return writeStr(start, end, "'\\f'");
          case 13: return writeStr(start, end, "'\\r'");
          default:
            start = writeChar(start, end, '\'');
            start = writeChar(start, end, '\\');
            start = writeInt(start, end, value.value);
            return writeChar(start, end, '\'');
        }
      }
      start = writeChar(start, end, '\'');
      // Encode the value as UTF-8 bytes
      if (value.value < 0x80) {
        start = writeChar(start, end, value.value);
      }
      else if (value.value < 0x800) {
        start = writeChar(start, end, 0xc0 | (value.value >> 6));
        start = writeChar(start, end, 0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x10000) {
        start = writeChar(start, end, 0xe0 | (value.value >> 12));
        start = writeChar(start, end, 0x80 | ((value.value >> 6) & 0x3f));
        start = writeChar(start, end, 0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x200000) {
        start = writeChar(start, end, 0xf0 | (value.value >> 18));
        start = writeChar(start, end, 0x80 | ((value.value >> 12) & 0x3f));
        start = writeChar(start, end, 0x80 | ((value.value >> 6) & 0x3f));
        start = writeChar(start, end, 0x80 | (value.value & 0x3f));
      }
      else {
        start = writeChar(start, end, '\\');
        start = writeChar(start, end, 'u');
        start = writeInt(start, end, value.value);
      }
      return writeChar(start, end, '\'');
    }
    case STRING: {
      buffer_t* buf = getBuffer(S, value);
      start = writeChar(start, end, '"');
      for (int i = 0; i < buf->length; i++) {
        uint8_t byte = buf->data[i];
        if (byte < 0x20) {
          switch (byte) {
            case 7: start = writeStr(start, end, "\\a"); break;
            case 8: start = writeStr(start, end, "\\b"); break;
            case 9: start = writeStr(start, end, "\\t"); break;
            case 10: start = writeStr(start, end, "\\n"); break;
            case 11: start = writeStr(start, end, "\\v"); break;
            case 12: start = writeStr(start, end, "\\f"); break;
            case 13: start = writeStr(start, end, "\\r"); break;
            default:
              start = writeChar(start, end, '\\');
              start = writeInt(start, end, byte);
          }
        }
        else if (byte == '"') {
          start = writeStr(start, end, "\\\"");
        }
        else {
          start = writeChar(start, end, byte);
        }
      }
      return writeChar(start, end, '"');
    }
    case SYMBOL: {
      buffer_t* buf = getBuffer(S, value);
      start = writeChar(start, end, ':');
      for (int i = 0; i < buf->length; i++) {
        uint8_t byte = buf->data[i];
        if (byte >= 0x20 && byte < 0x80) {
          start = writeChar(start, end, byte);
        }
      }
      return start;
    }
    case BYTE_ARRAY: {
      buffer_t* buf = getBuffer(S, value);
      start = writeChar(start, end, '<');
      for (int i = 0; i < buf->length; i++) {
        uint8_t byte = buf->data[i];
        if (i) start = writeChar(start, end, ' ');
        start = writeChar(start, end, toHex(byte >> 4 & 0xf));
        start = writeChar(start, end, toHex(byte >> 0 & 0xf));
      }
      return writeChar(start, end, '>');
    }
    case FRAME_BUFFER: {
      buffer_t* buf = getBuffer(S, value);
      uint32_t* data = (uint32_t*)buf->data;
      int32_t len = buf->length >> 2;
      start = writeChar(start, end, '<');
      for (int i = 0; i < len; i++) {
        uint32_t word = data[i];
        if (i) start = writeChar(start, end, ' ');
        start = writeChar(start, end, toHex(word >> 28 & 0xf));
        start = writeChar(start, end, toHex(word >> 24 & 0xf));
        start = writeChar(start, end, toHex(word >> 20 & 0xf));
        start = writeChar(start, end, toHex(word >> 16 & 0xf));
        start = writeChar(start, end, toHex(word >> 12 & 0xf));
        start = writeChar(start, end, toHex(word >> 8 & 0xf));
        start = writeChar(start, end, toHex(word >> 4 & 0xf));
        start = writeChar(start, end, toHex(word >> 0 & 0xf));
      }
      return writeChar(start, end, '>');
    }
    case PAIR: {
      pair_t pair = getPair(S, value);
      start = writeChar(start, end, '(');
      start = writeValue(start, end, S, pair.left);
      start = writeChar(start, end, ' ');
      start = writeValue(start, end, S, pair.right);
      return writeChar(start, end, ')');
    }
    case STACK: {
      start = writeChar(start, end, '[');
      pair_t pair = getPair(S, value);
      value_t next;
      bool later = false;
      while ((next = pair.right).type == PAIR) {
        pair = getPair(S, next);
        if (later) start = writeChar(start, end, ' ');
        later = true;
        start = writeValue(start, end, S, pair.left);
      }
      return writeChar(start, end, ']');
    }
    case SET: {
      start = writeChar(start, end, '|');
      bool later = false;
      start = writeSetNode(start, end, S, value, &later);
      return writeChar(start, end, '|');
    }
    case MAP: {
      start = writeChar(start, end, '{');
      bool later = false;
      start = writeMapNode(start, end, S, value, &later);
      return writeChar(start, end, '}');
    }
    case FUNCTION:
      return writeStr(start, end, "TODO: print FUNCTION");
    case CLOSURE:
      return writeStr(start, end, "TODO: print CLOSURE");
    case DEVICE:
      return writeStr(start, end, "TODO: print DEVICE");
  }
  return writeStr(start, end, "TODO");
}
