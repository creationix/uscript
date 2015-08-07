#include "uscript.h"

unsigned char setup_code[] = {
  PM, 13, OUTPUT,  // pinMode(LED_PIN1, OUTPUT)
  END
};

unsigned char loop_code[] = {
  DW, 13, NOT, DR, 13, // digitalWrite(13, !digitalRead(13))
  DELAY, 0x43, 0x74,   // delay(500)
};

void setup() {
  uscript_setup();
  uscript_run(setup_code);
}

void loop() {
  uscript_run(loop_code);
}