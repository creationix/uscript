#include "helpers.c"

// value_t String(state_t* S, int32_t length, const uint8_t* data);
// value_t Symbol(state_t* S, int32_t length, const uint8_t* data);
// value_t Buffer(state_t* S, int32_t length, const uint8_t* data);
// value_t Pixels(state_t* S, int32_t length, const uint32_t* data);
// buffer_t* getBuffer(state_t* S, value_t value);

int main() {
  state_t* S = State();
  value_t v, v2;

  v = String(S, -1, (uint8_t*)"Hello World");
  dump(S, v);
  assert(isBuffer(v));
  assert(v.type == STRING);
  buffer_t* b = getBuffer(S, v);
  assert(b->length == 11);
  assert(strncmp((char*)b->data, "Hello World", 11) == 0);

  v = Symbol(S, -1, (uint8_t*)"name");
  dump(S, v);
  assert(isBuffer(v));
  assert(v.type == SYMBOL);
  b = getBuffer(S, v);
  assert(b->length == 4);
  assert(strncmp((char*)b->data, "name", 4) == 0);

  v2 = Symbol(S, 4, (uint8_t*)"name");
  dump(S, v2);
  assert(isBuffer(v2));
  assert(v2.type == SYMBOL);
  assert(eq(v, v2));

  v = Buffer(S, 10, (uint8_t[]){
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa
  });
  dump(S, v);
  assert(isBuffer(v));
  assert(v.type == BYTE_ARRAY);
  b = getBuffer(S, v);
  assert(b->length == 10);
  assert(b->data[0] == 0x11);

  v = Pixels(S, 4, (uint32_t[]){
    0xdeadbeef,
    0x11223344,
    0x55667788,
    0x01010101,
  });
  dump(S, v);
  assert(isBuffer(v));
  assert(v.type == FRAME_BUFFER);
  b = getBuffer(S, v);
  assert(b->length == 4 * 4);

  freeState(S);
  return 0;
}
