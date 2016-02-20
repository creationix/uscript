#include "dump.h"
#include <stdio.h>
#include <inttypes.h>

static void printSetNode(state_t* S, value_t node, bool* later) {
  if (falsy(node)) return;
  pair_t pair = getPair(S, node);
  node = pair.left;
  if (truthy(node)) {
    if (*later) putchar(' ');
    prettyPrint(S, node);
    *later = true;
  }
  node = pair.right;
  if (falsy(node)) return;
  pair_t split = getPair(S, node);
  printSetNode(S, split.right, later);
  printSetNode(S, split.left, later);
}

static void printMapNode(state_t* S, value_t node, bool* later) {
  if (falsy(node)) return;
  pair_t pair = getPair(S, node);
  if (pair.left.type == PAIR) {
    if (*later) putchar(' ');
    pair_t mapping = getPair(S, pair.left);
    prettyPrint(S, mapping.left);
    putchar(':');
    prettyPrint(S, mapping.right);
    *later = true;
  }
  node = pair.right;
  if (falsy(node)) return;
  pair_t split = getPair(S, node);
  printMapNode(S, split.right, later);
  printMapNode(S, split.left, later);
}


void prettyPrint(state_t* S, value_t value) {
  switch (value.type) {
    case BOOLEAN:
      printf("%s", value.value ? "true" : "false");
      break;
    case INTEGER:
    case BOX_INTEGER:
      printf("%"PRId64, toInt(S, value));
      break;
    case RATIONAL: {
      rational_t r = getRational(S, value);
      if (r.dem == 0) {
        if (r.num > 0) printf("Inf");
        else if (r.num < 0) printf("-Inf");
        else printf("NaN");
        break;
      }
      printf("%"PRId64, r.num);
      putchar('/');
      printf("%"PRId64, r.dem);
      break;
    }
    case CHARACTER:
      putchar('\'');
      if (value.value < 0x20) {
        switch (value.value) {
          case 7: printf("\\a"); break;
          case 8: printf("\\b"); break;
          case 9: printf("\\t"); break;
          case 10: printf("\\n"); break;
          case 11: printf("\\v"); break;
          case 12: printf("\\f"); break;
          case 13: printf("\\r"); break;
          default: printf("\\%d", value.value); break;
        }
      }
      // Encode the value as UTF-8 bytes
      else if (value.value < 0x80) {
        putchar((signed)value.value);
      }
      else if (value.value < 0x800) {
        putchar(0xc0 | (value.value >> 6));
        putchar(0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x10000) {
        putchar(0xe0 | (value.value >> 12));
        putchar(0x80 | ((value.value >> 6) & 0x3f));
        putchar(0x80 | (value.value & 0x3f));
      }
      else if (value.value < 0x200000) {
        putchar(0xf0 | (value.value >> 18));
        putchar(0x80 | ((value.value >> 12) & 0x3f));
        putchar(0x80 | ((value.value >> 6) & 0x3f));
        putchar(0x80 | (value.value & 0x3f));
      }
      else {
        printf("\\u%d", value.value);
      }
      putchar('\'');
      break;
    case STRING: {
      buffer_t* buf = getBuffer(S, value);
      printf("\"%.*s\"", buf->length, buf->data);
      break;
    }
    case SYMBOL: {
      buffer_t* buf = getBuffer(S, value);
      printf(":%.*s", buf->length, buf->data);
      break;
    }
    case BYTE_ARRAY: {
      buffer_t* buf = getBuffer(S, value);
      putchar('<');
      for (int i = 0; i < buf->length; i++) {
        if (i) putchar(' ');
        printf("%02x", buf->data[i]);
      }
      putchar('>');
      break;
    }
    case FRAME_BUFFER: {
      buffer_t* buf = getBuffer(S, value);
      uint32_t* data = (uint32_t*)buf->data;
      int32_t len = buf->length >> 2;
      putchar('<');
      for (int i = 0; i < len; i++) {
        if (i) putchar(' ');
        printf("%08x", data[i]);
      }
      putchar('>');
      break;
    }
    case PAIR: {
      pair_t pair = getPair(S, value);
      putchar('(');
      prettyPrint(S, pair.left);
      putchar(' ');
      prettyPrint(S, pair.right);
      putchar(')');
      break;
    }
    case STACK: {
      putchar('[');
        pair_t pair = getPair(S, value);
        value_t next;
        bool later = false;
        while (truthy(next = pair.right)) {
          pair = getPair(S, next);
          if (later) putchar(' ');
          later = true;
          prettyPrint(S, pair.left);
        }
      }
      putchar(']');
      break;
    case SET: {
      putchar('|');
      bool later = false;
      printSetNode(S, value, &later);
      putchar('|');
      break;
    }
    case MAP: {
      putchar('{');
      bool later = false;
      printMapNode(S, value, &later);
      putchar('}');
      break;
    }
    case FUNCTION:
      printf("TODO: print FUNCTION");
      break;
    case CLOSURE:
      printf("TODO: print CLOSURE");
      break;
    case DEVICE:
      printf("TODO: print DEVICE");
      break;
  }
}

void dump(state_t* S, value_t value) {
  prettyPrint(S, value);
  putchar('\n');
}
