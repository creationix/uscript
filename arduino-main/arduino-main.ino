
#define BUFFER_LENGTH 512


#include "uscript.c"

void write_string(const char* str) {
  Serial.print(str);
}
void write_number(number num) {
  Serial.print(num);
}
void write_char(char c) {
  Serial.write(c);
}

static struct state vm;

void setup() {
  vm.write_string = write_string;
  vm.write_number = write_number;
  vm.write_char = write_char;
  Serial.begin(9600);
  start_state(&vm);
}

void loop() {
  while (Serial.available() > 0) handle_input(&vm, Serial.read());
}
