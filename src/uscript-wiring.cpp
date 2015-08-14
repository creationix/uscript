#include "uscript-wiring.h"

void printStr(const char* str);
void printNum(int str);

void Dump(struct state* s) {
  intptr_t* i = s->stack;
  Serial.print("stack:");
  for (i = s->stack; i <= s->top; i++) {
    Serial.print(" ");
    Serial.print(*i);
  }
  Serial.print("\r\n");
  DONEXT
}
void Delay(struct state* s) {
  delay(*s->top--);
  DONEXT
}
void DelayMicroseconds(struct state* s) {
  delayMicroseconds(*s->top--);
  DONEXT
}
void PinMode(struct state* s) {
  pinMode(*(s->top - 1), *s->top);
  s->top -= 2;
  DONEXT;
}
void DigitalWrite(struct state* s) {
  digitalWrite(*(s->top - 1), *s->top);
  s->top -= 2;
  DONEXT;
}
void DigitalRead(struct state* s) {
  *s->top = digitalRead(*s->top);
  DONEXT;
}
void AnalogWrite(struct state* s) {
  analogWrite(*(s->top - 1), *s->top);
  s->top -= 2;
  DONEXT;
}
void AnalogRead(struct state* s) {
  *s->top = analogRead(*s->top);
  DONEXT;
}

void Tone(struct state* s) {
  s->top -= 3;
  long int pin = *(s->top + 1),
           frequency = *(s->top + 2);
  long int duration = *(s->top + 3);
  long int p = 1000000 / frequency;
  long int us = p >> 1;
  duration *= 1000;
  while ((duration -= p) > 0) {
    digitalWrite(pin, 1);
    delayMicroseconds(us);
    digitalWrite(pin, 0);
    delayMicroseconds(us);
  }
  DONEXT
}
