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
#include <vector>
#include "esp_deep_sleep.h"
#include "dht.h"
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "mbedtls/net.h"

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up
#define SENSOR_PIN GPIO_NUM_26


uint64_t usecNow()
{
    timeval t;
    gettimeofday(&t, nullptr);
    return static_cast<uint64_t>( t.tv_sec ) * 1000000ull + static_cast<uint64_t>( t.tv_usec );
}

void sendSensorInit()
{
    gpio_set_level(SENSOR_PIN,0);
    ets_delay_us(1000); //1ms low T-be
    gpio_set_level(SENSOR_PIN,1);
    ets_delay_us(30); //30 usec high T-go
    gpio_set_level(SENSOR_PIN,0); //Bring the bus down, wait for sensor
}



void getSensorAck()
{
    //Sensor ack starts with zero and zero lasts T-rel usec
    int32_t rel_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 0)
        ++rel_cnt;

    //Now we are in high and wait for low
    int32_t reh_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 1)
        ++reh_cnt;
    printf("ACK T-rel: %i T-reh: %i\n",rel_cnt,reh_cnt);
}

int64_t readBit()
{
    int32_t low_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 0)
        ++low_cnt;
    int32_t high_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 1)
        ++high_cnt;
    printf("Bit: low: %i high: %i\n",low_cnt, high_cnt);
    return high_cnt;
}

bool readPin(int& value, int& duration,int timeout)
{
    uint64_t now = usecNow();
    int cntx = 0;
    value = gpio_get_level(SENSOR_PIN);
    while(gpio_get_level(SENSOR_PIN) == value && cntx < timeout)
        ++cntx;
    duration = (int) (usecNow() - now);
    return cntx < timeout;
}

typedef struct tagValTime
{
    tagValTime() : value(0), duration(0){}
    int value;
    int duration;
} ValTime;

//mbedtls_net_usleep(25);
extern "C" void app_main()
{
    sys_init();

    gpio_intr_disable(SENSOR_PIN);
    gpio_wakeup_disable(SENSOR_PIN);
    gpio_set_direction(SENSOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SENSOR_PIN,1);
    sys_msleep(10);

    std::vector<ValTime> values;
    values.reserve(128);



    printf("Init:\n");
    sendSensorInit();
    gpio_set_direction(SENSOR_PIN, GPIO_MODE_INPUT);

    ValTime val;

    while(readPin(val.value,val.duration,10000))
    {
        values.push_back(val);
        val.value = 0;
        val.duration = 0;
    }

    printf("Read %i values.\n",values.size());
    for(int i = 0; i < values.size(); i++)
        printf("%i:  Value: %i Duration: %i.\n",i, values[i].value,values[i].duration);


//    printf("Ack:\n");
//    getSensorAck();
//    for(int i = 0; i < 40; i++)
//    {
//        readBit();
//    }
//
//    xTaskGetTickCount();
//    sys_now();
//    printf("deep sleep\n");
//    esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
}
