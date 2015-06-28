#include "uscript.c"


void setup() {
  Serial.begin(9600);
  Serial.print("> ");
}

#define MAX_LEN 140
static uint8_t line[MAX_LEN + 1];
int offset = 0;

void loop() {

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (offset < MAX_LEN && c >= 0x20 && c < 0x7f) {
      line[offset++] = c;
      Serial.write(c);
    }
    else if (c == 127 or c == 8) {
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
            int32_t result;
            offset = eval(line + offset, &result) - line;
            Serial.println(result);
          }
        }

      }
      Serial.write("> ");
      offset = 0;
    }
  }
}
