#include <Arduino.h>

void setup() {
  Serial.begin(11520);
}

void loop() {
  delay(1000);
  Serial.println("Hello World!");
}
