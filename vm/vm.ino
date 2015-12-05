#include <ESP8266WiFi.h>
#include <string.h>
#include "vm.h"

const char* ssid     = "creationix-mobile";
const char* password = "noderocks";
const char* host = "192.168.43.221";

/*const char* ssid     = "rackbook";
const char* password = "159978e273";
const char* host = "10.42.0.1";*/

WiFiClient client;

int isAlive() {
  return client.connected() && !client.available();
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

int mode = 1;
uint8_t* code;
uint8_t* pc;
int len;
void handle(char c) {
  Serial.print("handle char: ");
  Serial.println((int)c);
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
      pc = eval(code, &result);
      free(code);
      mode = 1;
    }
  }
}

int second = 0;
void loop() {

  if (!second || !client.connected()) {
    if (second) {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
    second = 1;
    delay(1000);
    Serial.print("connecting to ");
    Serial.println(host);
    while (!client.connect(host, 1337)) {
      Serial.println("connection failed");
      delay(1000);
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

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    handle(client.read());
  }
}
