#include "uscript.h"

int slots[16];
void printString(const char* str) {
  Serial.print(str);
}
void printInt(int val) {
  Serial.print(val);
}
void slotWrite(int addr, int slot, int value) {
  Serial.print("TODO: send message to remote slot\r\n");
}

int slotRead(int i) {
  return slots[i];
}

#define CLK 8
#define SDA 9

#define Int16(val) ((uint16_t)(val) & 0xff), ((uint16_t)(val) >> 8)

unsigned char code[] = {
  // Setup
  13, OUTPUT, PM,  // pinMode(13, OUTPUT)
  SDA, 1, PM, SDA, 0, DW, // Set data line to pull low
  CLK, 1, PM, CLK, 0, DW, // Set clock line to pull low
  END,
  // Loop
  13, 13, DR, NOT, DW, // digitalWrite(13, !digitalRead(13))
  0x43, 0x74, DELAY,   // delay(500)
  END,
  // Start
  SDA, 1, PM, // Pull data line low
  50, UDELAY, // Wait 50 us
  CLK, 1, PM, // Pull clock low
  END,
  // Stop
  END,
  // Send byte 
  DUP, 1, BAND, CALL, 0x10, Uint16(53),
  DUP, 2, BAND, CALL, 0x10, Uint16(46),
  DUP, 4, BAND, CALL, 0x10, Uint16(38),
  DUP, 8, BAND, CALL, 0x10, Uint16(31),
  DUP, 16, BAND, CALL, 0x10, Uint16(24),
  DUP, 32, BAND, CALL, 0x10, Uint16(17),
  DUP, 0x40, 0x40, BAND, CALL, 0x10, Uint16(9),
  DUP, 0x41, 0x00, BAND, CALL, 0x10, Uint16(1),
  END,
  // Send bit
  CLK, 1, PM, // Set clock line to pull low
  SDA, OVER, NOT, PM, 
  CLK, 0, PM, // Release clock line
  END,
  // read byte
  END,
};

void setup() {
  Serial.begin(9600);
  eval(code);
}

void loop() {
  reset();
  eval(code + 4);
}

