#include <SoftwareSerial.h>

SoftwareSerial espSerial(10, 11);  // RX = 10 (ESP32 TX接続)

void setup() {
  Serial.begin(9600);         // PCとの通信
  espSerial.begin(9600);      // ESP32との通信
}

void loop() {
  if (espSerial.available()) {
    String line = espSerial.readStringUntil('\n');
    Serial.println("Received: " + line);
  }
}
