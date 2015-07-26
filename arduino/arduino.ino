#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <EEPROM.h>
#define REPL_BUFFER 4096
#define EEPROM_SIZE 4096
#define CHECKER (yield(),!digitalRead(0))
#include "uscript.c"

static uint8_t* ICACHE_FLASH_ATTR PinMode(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, mode;
  pc = eval(S, eval(S, pc, &pin), &mode);
  pinMode(pin, mode);
  *res = mode;
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR DigitalWrite(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, val;
  pc = eval(S, eval(S, pc, &pin), &val);
  digitalWrite(pin, val);
  *res = val;
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR DigitalRead(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  *res = digitalRead(pin);
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR AnalogWrite(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, skip(S, pc));
  number pin, val;
  pc = eval(S, eval(S, pc, &pin), &val);
  analogWrite(pin, val);
  *res = val;
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR AnalogRead(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, pc);
  number pin;
  pc = eval(S, pc, &pin);
  *res = analogRead(pin);
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR Tone(struct uState* S, uint8_t* pc, number* res) {
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

static uint8_t* ICACHE_FLASH_ATTR Delay(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  delay(*res);
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR Print(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return skip(S, pc);
  pc = eval(S, pc, res);
  Serial.print(*res);
  Serial.print("\r\n");
  return pc;
}

static uint8_t* ICACHE_FLASH_ATTR Save(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return pc;
  printf("TODO: save to EEPROM\n");
  return pc;
}

static void ICACHE_FLASH_ATTR dump(struct uState* S, uint8_t* pc, int len) {
  uint8_t* end = pc + len;
  while (pc < end) {
    Serial.write(' ');
    // If the high bit is set, it's an opcode index.
    if (*pc & 0x80) {
      Serial.print(op_to_name(S, *pc++));
      continue;
    }
    // Otherwise it's a variable length encoded integer.
    number val = *pc & 0x3f;
    if (*pc++ & 0x40) {
      int b = 6;
      do {
        val |= (number)(*pc & 0x7f) << b;
        b += 7;
      } while (*pc++ & 0x80);
    }
    Serial.print(val);
  }
}

static uint8_t* ICACHE_FLASH_ATTR List(struct uState* S, uint8_t* pc, number* res) {
  if (!res) return pc;
  int i;
  *res = 2;
  for (i = 0; i < SIZE_STUBS; i++) {
    if (!S->stubs[i]) continue;
    Serial.print("DEF ");
    Serial.write(i + 'a');
    int len = skip(S, S->stubs[i]) - S->stubs[i];
    dump(S, S->stubs[i], len);
    Serial.print("\r\n");
    *res += len + 3;
  }
  return pc;
}

static struct uState S;

static struct user_func funcs[] = {
  {"PM", PinMode},      // (pin, mode)
  {"DW", DigitalWrite}, // (pin, value)
  {"DR", DigitalRead},  // (pin)
  {"AW", AnalogWrite}, // (pin, value)
  {"AR", AnalogRead},  // (pin)
  {"TONE", Tone},       // (pin, frequency, duration)
  {"DELAY", Delay},     // (ms)
  {"PRINT", Print},     // (num)
  {"LIST", List},       //
  {"SAVE", Save},       //
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

  S.malloc = malloc;
  S.free = free;
  S.funcs = funcs;
  S.num_funcs = 0;
  while (funcs[S.num_funcs].name) S.num_funcs++;

  Serial.begin(9600);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.print("\r\nWelcome to uscript.\r\n");
  #ifdef ARDUINO
    pinMode(0, 0);
    EEPROM.begin(EEPROM_SIZE);
    int o = 0;
    if (EEPROM.read(o++) == 'u') {
      while (EEPROM.read(o) != 'u') {
        int key = EEPROM.read(o++);
        Serial.print("Loading ");
        Serial.write(key + 'a');
        Serial.print("...\r\n");
        int len = EEPROM.read(o++) << 8;
        len |= EEPROM.read(o++);
        uint8_t* stub = (uint8_t*)malloc(len);
        S.stubs[key] = stub;
        int j;
        for (j = 0; j < len; j++) {
          stub[j] = EEPROM.read(o++);
        }
      }
    }
    EEPROM.end();
  #endif
  if (S.stubs[0]) {
    Serial.print("Running auto script...\r\n");
    number out;
    eval(&S, S.stubs[0], &out);
  }
  Serial.print("> ");

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

uint8_t line[REPL_BUFFER];
int offset;

void handle_input(char c) {
  if (offset < REPL_BUFFER && c >= 0x20 && c < 0x7f) {
    line[offset++] = c;
    Serial.write(c);
  }
  else if (offset > 0 && (c == 127 || c == 8)) {
    line[--offset] = 0;
    Serial.print("\x08 \x08");
  }
  else if (c == '\r' || c == '\n') {
    Serial.print("\r\n");
    if (offset) {
      line[offset++] = 0;
      int len = compile(&S, line);
      if ((int) len < 0) {
        int offset = 1 - (int)len;
        while (offset--) Serial.print(" ");
        Serial.print("^\r\nUnexpected input\r\n");
      }
      else {
        int offset = 0;
        while (offset < len) {
          number result;
          offset = eval(&S, line + offset, &result) - line;
          Serial.print(result);
          Serial.print("\r\n");
        }
      }

    }
    offset = 0;
    Serial.print("> ");
  }
}

void loop() {
  webSocket.loop();
  while (Serial.available() > 0) handle_input(Serial.read());
}
