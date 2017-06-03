/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>
#include <dht.h>

#define DHT22_PIN 40

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.print("Initialized Blink\n");
}
dht DHT;
void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(2000);
  Serial.print("Blinked...\n");

  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
   // wait for a second
  delay(1000);
  // READ DATA
Serial.print("DHT22, \t");
int chk = DHT.read22(DHT22_PIN);
switch (chk)
{
  case DHTLIB_OK:
  Serial.print("OK,\t");
  break;
  case DHTLIB_ERROR_CHECKSUM:
  Serial.print("Checksum error,\t");
  break;
  case DHTLIB_ERROR_TIMEOUT:
  Serial.print("Time out error,\t");
  break;
  default:
  Serial.print("Unknown error,\t");
  break;
}
// DISPLAY DATA
Serial.print(DHT.humidity, 1);
Serial.print(",\t");
Serial.println(DHT.temperature, 1);
}
