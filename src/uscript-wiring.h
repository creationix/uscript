#ifndef USCRIPT_WIRING_H
#define USCRIPT_WIRING_H
#include "uscript.h"
#include "Arduino.h"

void Dump(struct state* s);
void Delay(struct state* s);
void DelayMicroseconds(struct state* s);
void PinMode(struct state* s);
void DigitalWrite(struct state* s);
void DigitalRead(struct state* s);
void AnalogWrite(struct state* s);
void AnalogRead(struct state* s);
void Tone(struct state* s);

#endif
