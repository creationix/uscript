#include "utils.h"

#include <string.h>

static value_t RawBuffer(state_t* S, type_t type, int32_t length, const uint8_t* data) {
  if (data && length < 0) {
    length = (int32_t)strlen((char*)data);
  }
  if (length + 4 > S->num_bytes - S->next_bytes) {
    // TODO: GC and/or resize space
    return Bool(false);
  }
  int32_t start = S->next_bytes;
  buffer_t* buf = (buffer_t*)(S->bytes + start);
  buf->gc = true;
  buf->length = length;
  if (data) {
    memcpy(buf->data, data, (size_t)length);
  }
  else {
    memset(buf->data, 0, (size_t)length);
  }
  S->next_bytes += length + 4;
  return (value_t){
    .gc = true,
    .type = type,
    .value = start
  };
}

value_t String(state_t* S, int32_t len, const uint8_t* str) {
  if (len < 0) {
    len = (int32_t)strlen((const char*)str);
  }
  return RawBuffer(S, STRING, len, str);
}

value_t Symbol(state_t* S, int32_t len, const uint8_t* str) {
  if (len < 0) {
    len = (int32_t)strlen((const char*)str);
  }
  uint8_t* offset = S->bytes;
  while (true) {
    buffer_t* buf = (buffer_t*)offset;
    // If we reach the end, it's not here, just add a new buffer.
    if (offset > S->bytes + S->num_bytes || !(buf->gc || buf->length)) {
      return RawBuffer(S, SYMBOL, len, (uint8_t*)str);
    }
    // If we find a matching string, reuse the existing value.
    if (buf->gc && buf->length == len && !strcmp((char*)buf->data, (const char*)str)) {
      return (value_t){
        .gc = 1,
        .type = SYMBOL,
        .value = (int32_t)(offset - S->bytes)
      };
    }
    offset += buf->length + 4;
  }
}

value_t Buffer(state_t* S, int32_t length, const uint8_t* data) {
  if (length < 0) return Bool(false);
  return RawBuffer(S, BYTE_ARRAY, length, data);
}

value_t Pixels(state_t* S, int32_t length, const uint32_t* data) {
  if (length < 0) return Bool(false);
  return RawBuffer(S, FRAME_BUFFER, length << 3, (const uint8_t*)data);
}
