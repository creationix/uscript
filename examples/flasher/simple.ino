#include "uscript.h"

#define LED_PIN1 2
#define LED_PIN2 6
#define LED_PIN3 A1
#define INPUT_PIN A0

unsigned char code1[] = {
  PM, LED_PIN1, OUTPUT,  // pinMode(LED_PIN1, OUTPUT)
  PM, LED_PIN2, OUTPUT,  // pinMode(LED_PIN2, OUTPUT)
  PM, LED_PIN3, OUTPUT,  // pinMode(LED_PIN3, OUTPUT)
  PM, INPUT_PIN, INPUT,  // pinMode(INPUT_PIN, INPUT)
  END
};

unsigned char code2[] = {
  W1, AR, INPUT_PIN,
  W0, DR, LED_PIN1,      // a = digitalRead(LED_PIN1)
  PRINT, R0,             // Serial.println(a)
  DW, LED_PIN1, NOT, R0, // digitalWrite(LED_PIN1, !a)
  DW, LED_PIN2, R0,      // digitalWrite(LED_PIN1, a)
  DW, LED_PIN3, NOT, DR, LED_PIN3,
  DELAY, R1,     // delay(b)
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
