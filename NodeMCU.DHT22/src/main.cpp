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
#include "sdkconfig.h"
#include "esp_deep_sleep.h"
#include "DHT22.h"

extern "C" void app_main()
{
    DHT22 sensor(GPIO_NUM_26);
    int errorCode = 0;
    for(int i = 0; i < 20; i++)
        if((errorCode = sensor.readSensor()) != 0)
            printf("Read sensor error: %i\n",errorCode);
        else
            break;

    printf("Humidity: %f. Temp: %f.\n",sensor.humidity(),sensor.temperature());


    esp_deep_sleep(1000000LL * 10); //10s deep sleep
}
