#include "uscript.c"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  int32_t res;
  Serial.print("1 + 2 = ");
  const uint8_t program[] = { OP_ADD, 1, 2 };
  eval(program, &res);
  Serial.println(res);
  delay(1000);

}
