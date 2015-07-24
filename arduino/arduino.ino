#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <EEPROM.h>
#include "uscript.h"

static unsigned char* ICACHE_FLASH_ATTR PinMode(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, mode;
  pc = eval(S, eval(S, pc, &pin), &mode);
  pinMode(pin, mode);
  *res = mode;
  return pc;
}

static unsigned char* ICACHE_FLASH_ATTR DigitalWrite(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, val;
  pc = eval(S, eval(S, pc, &pin), &val);
  digitalWrite(pin, val);
  *res = val;
  return pc;
}

static unsigned char* ICACHE_FLASH_ATTR DigitalRead(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  *res = digitalRead(pin);
  return pc;
}

static unsigned char* ICACHE_FLASH_ATTR Tone(struct uState* S, unsigned char* pc, number* res) {
  if (!res) return skip(S, skip(S, skip(S, pc)));
  number pin, frequency, duration;
  pc = eval(S, eval(S, eval(S, pc, &pin), &frequency), &duration);

  int p = 1000000 / frequency;
  int us = p >> 1;
  duration *= 1000;
  while ((duration -= p) > 0) {
    digitalWrite(pin, 1);
    delayMicroseconds(us);
    digitalWrite(pin, 0);
    delayMicroseconds(us);
  }
  *res = frequency;
  return pc;
}


static struct uState S;

static struct user_func funcs[] = {
  {"PM", PinMode},      // (pin, mode)
  {"DW", DigitalWrite}, // (pin, value)
  {"DR", DigitalRead},  // (pin)
  {"TONE", Tone},       // (pin, frequency, duration)
  {NULL},
};



ESP8266WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(80);

String buffer;
int useBuffer;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      int i;
      webSocket.sendBIN(num, (uint8_t*)"defs", 4);
      for (i = 0; i < SIZE_STUBS; i++) {
        if (!S.stubs[i]) continue;
        uint8_t name = i + 'a';
        webSocket.sendBIN(num, &name, 1);
        int len = skip(&S, S.stubs[i]) - S.stubs[i];
        webSocket.sendBIN(num, S.stubs[i], len);
      }
      webSocket.sendBIN(num, (uint8_t*)"end", 3);
      break;
    }
    case WStype_TEXT:
      if (!(length && payload)) return;
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      length = compile(&S, payload);
      // Fall-through on purpose
    case WStype_BIN:
      if (!(length && payload)) return;
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      number out;
      buffer = String();
      useBuffer = 1;
      eval(&S, payload, &out);
      useBuffer = 0;
      buffer = buffer + out;
      webSocket.sendTXT(num, buffer);
      break;
  }
}


static void on_write_string(const char* str) {
  if (useBuffer) buffer = buffer + str;
  else Serial.print(str);
}
static void on_write_number(int32_t num) {
  if (useBuffer) buffer = buffer + num;
  else Serial.print(num);
}
static void on_write_char(char c) {
  if (useBuffer) buffer = buffer + c;
  else Serial.write(c);
}

void setup() {

  Serial.begin(9600);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);



  // Serial.print("\r\nConnecting to access point");
  // WiFiMulti.addAP("creationix", "noderocks");
  // while(WiFiMulti.run() != WL_CONNECTED) {
  //   Serial.print(".");
  //   delay(100);
  // }
  //
  // Serial.print("\r\nLocal IP: ");
  // Serial.println(WiFi.localIP());

}

void loop() {
  webSocket.loop();
  while (Serial.available() > 0) handle_input(Serial.read());
}
