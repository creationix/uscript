
#include "uscript.h"
static void on_write_string(const char* str) {
  Serial.print(str);
}
static void on_write_number(int32_t num) {
  Serial.print(num);
}
static void on_write_char(char c) {
  Serial.write(c);
}

void setup() {
  write_string = on_write_string;
  write_number = on_write_number;
  write_char = on_write_char;
  Serial.begin(9600);
  start();
}

void loop() {
  while (Serial.available() > 0) handle_input(Serial.read());
}
