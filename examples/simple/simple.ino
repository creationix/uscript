#include "uscript.h"

unsigned char code1[] = {
  PM, 13, 1,         // pinMode(13, OUTPUT)
  END
};

unsigned char code2[] = {
  W0, DR, 13,        // a = digitalRead(13)
  PRINT, R0,         // Serial.println(a)
  DW, 13, NOT, R0,   // digitalWrite(14, !a)
  DELAY, 0x43, 0x74, // delay(500)
  END
};

void setup() {
  Serial.begin(9600);
  uscript_setup();
  uscript_run(code1);
}

void loop() {
  uscript_run(code2);
}
