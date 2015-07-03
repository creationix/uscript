
#define MAX_LEN 140

#include "uscript.c"

static char read_char() {
  return Serial.read();
}
static void write_string(const char* string) {
  return Serial.write(string);
}
static void write_val(var val) {
  return Serial.write(val);
}

void setup() {
  Serial.begin(9600);
  Serial.print("> ");
  print_fn = &myprint;
}

static uint8_t line[MAX_LEN + 1];
int offset = 0;

void loop() {

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (offset < MAX_LEN && c >= 0x20 && c < 0x7f) {
      line[offset++] = c;
      Serial.write(c);
    }
    else if (offset > 0 && (c == 127 or c == 8)) {
      line[--offset] = 0;
      Serial.write("\x08 \x08");
    }
    else if (c == '\r' || c == '\n') {
      Serial.println("");
      if (offset) {
        line[offset++] = 0;
        int len = compile(line);
        if ((int) len < 0) {
          int offset = 1 - (int)len;
          while (offset--) Serial.write(" ");
          Serial.println("^");
          Serial.println("Unexpected input");
        }
        else {
          int offset = 0;
          while (offset < len) {
            var result;
            offset = eval(line + offset, &result) - line;
            Serial.println(result);
          }
        }

      }
      offset = 0;
      Serial.write("> ");
    }
  }

}
