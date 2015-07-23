#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <EEPROM.h>
#include "uscript.h"


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
      for (i = 0; i < NUM_STUBS; i++) {
        if (!stubs[i]) continue;
        uint8_t name = i + 'a';
        webSocket.sendBIN(num, &name, 1);
        int len = skip(stubs[i]) - stubs[i];
        webSocket.sendBIN(num, stubs[i], len);
      }
      webSocket.sendBIN(num, (uint8_t*)"end", 3);
      break;
    }
    case WStype_TEXT:
      if (!(length && payload)) return;
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      length = compile(payload);
      // Fall-through on purpose
    case WStype_BIN:
      if (!(length && payload)) return;
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      number out;
      buffer = String();
      useBuffer = 1;
      eval(payload, &out);
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

  write_string = on_write_string;
  write_number = on_write_number;
  write_char = on_write_char;
  start();


  Serial.print("\r\nConnecting to access point");
  WiFiMulti.addAP("creationix", "noderocks");
  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.print("\r\nLocal IP: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  webSocket.loop();
  while (Serial.available() > 0) handle_input(Serial.read());
}
