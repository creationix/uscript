#include <ESP8266WiFi.h>
#include <string.h>
#include "vm.h"

#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

const char* host = "creationix.com";
const int port = 7001;


WiFiClient client;


void setup() {
  Serial.begin(115200);
  delay(10);

  wifiMulti.addAP("creationix", "noderocks");
  wifiMulti.addAP("creationix-mobile", "noderocks");
  wifiMulti.addAP("WIN_AE2F", "SDPMSEX2");

  // We start by connecting to a WiFi network

  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

int mode = 1;
uint8_t* code;
uint8_t* pc;
int len;
void handle(char c) {
  if (mode == 1) {
    len = c << 8;
    mode = 2;
  }
  else if (mode == 2) {
    len |= c;
    mode = 3;
    code = (uint8_t*)malloc(len);
    pc = code;
    Serial.print("Got length header: ");
    Serial.println(len);
  }
  else {
    *pc++ = c;
    if (!--len) {
      Serial.println("Runing code");
      int32_t result;
      int32_t stack[100];
      pc = eval(stack, code, &result);
      free(code);
      mode = 1;
    }
  }
}

int second = 0;
int cooldown;
void maintainConnection() {
  if(wifiMulti.run() != WL_CONNECTED) {
      Serial.println("WiFi not connected!");
      delay(1000);
      return;
  }
  
  if (!second || !client.connected()) {
    if (millis() < cooldown) return;
    if (second) client.stop();
    second = 1;
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, port)) {
      Serial.println("connection failed");
      cooldown = millis() + 1000;
      return;
    }
    Serial.println("Connected!");
    // Send the chip ID and protocol version as 5-bytes
    int id = ESP.getChipId();
    client.write((char) USCRIPT_VERSION);
    client.write((char) (id >> 24) & 0xff);
    client.write((char) (id >> 16) & 0xff);
    client.write((char) (id >> 8) & 0xff);
    client.write((char) id & 0xff);
  }
}

int isAlive() {
  maintainConnection();
  yield();
  return !client.available();
}

void loop() {

  maintainConnection();
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    handle(client.read());
  }
}
