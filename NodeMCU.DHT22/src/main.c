/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys/time.h"

#include "sdkconfig.h"

#include "esp_deep_sleep.h"
#include "dht.h"

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory

void app_main() {
	dht sensor(4);

	Serial.print("DHT22, \t");
int chk = sensor.read22();
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
Serial.print(sensor.humidity, 1);
Serial.print(",\t");
Serial.println(sensor.temperature, 1);

esp_deep_sleep_enable_timer_wakeup(1000000LL * GPIO_DEEP_SLEEP_DURATION); // set timer but don't sleep now
printf("deep sleep\n");
esp_deep_sleep_start();

}
