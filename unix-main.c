#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "uv.h"

#include "uscript.c"

static struct state vm;
static uv_loop_t loop;
static uv_tty_t in;
static uv_tty_t out;

void write_string(const char* str) {
  uv_buf_t buf = { .base = (char*)str, .len = strlen(str) };
  uv_try_write((uv_stream_t*)&out, &buf, 1);
}
void write_number(number num) {
  char str[15];
  sprintf(str, "%"PRId64, num);
  return write_string(str);
}
void write_char(char c) {
  char s[] = {c};
  uv_buf_t buf = { .base = s, .len = 1 };
  uv_try_write((uv_stream_t*)&out, &buf, 1);
}

static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread == 0) return;
  if (nread < 0) {
    printf("Read error\n");
    return exit(nread);
  }
  if (nread == 1 && *buf->base == 4) {
    uv_read_stop((uv_stream_t*)&in);
    return;
  }

  int i;
  for (i = 0; i < nread; i++) {
    handle_input(&vm, buf->base[i]);
  }
}

static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->len = suggested_size;
  buf->base = malloc(suggested_size);
}

int main() {
  #ifdef BCM2708_PERI_BASE
    setup_io();
  #endif

  vm.write_string = write_string;
  vm.write_char = write_char;
  vm.write_number = write_number;

  uv_loop_init(&loop);
  uv_tty_init(&loop, &in, 0, 1);
  uv_tty_set_mode(&in, UV_TTY_MODE_RAW);

  uv_tty_init(&loop, &out, 1, 0);
  // uv_stream_set_blocking((uv_stream_t*)&out, 1);

  uv_read_start((uv_stream_t*)&in, on_alloc, on_read);

  start_state(&vm);

  uv_run(&loop, UV_RUN_DEFAULT);
  write_string("\r\n");

  uv_tty_reset_mode();
}
