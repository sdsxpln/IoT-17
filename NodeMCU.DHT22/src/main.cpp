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

void sendSensorInit()
{
    gpio_set_level(SENSOR_PIN,0);
    ets_delay_us(1000); //1ms low T-be
    gpio_set_level(SENSOR_PIN,1);
    ets_delay_us(30); //30 usec high T-go
    gpio_set_level(SENSOR_PIN,0); //Bring the bus down, wait for sensor
}

void readAck()
{
    //Sensor ack starts with zero and zero lasts T-rel usec
    int32_t rel_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 0)
        ++rel_cnt;

    //Now we are in high and wait for low
    int32_t reh_cnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 1)
        ++reh_cnt;
}

int readBit(int timeout)
{
    int lowValueCnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 0 && lowValueCnt < timeout)
        ++lowValueCnt;

    if(lowValueCnt == timeout)
        return -1;

    int highValueCnt = 0;
    while(gpio_get_level(SENSOR_PIN) == 1 && highValueCnt < timeout)
        ++highValueCnt;

    if(highValueCnt == timeout)
        return -1;

    if(highValueCnt < lowValueCnt*0.9)
        return 0;
    else if(highValueCnt > lowValueCnt*1.1)
        return 1;

    return -1;
}

bool readPin(int& value, int& duration,int timeout)
{
    duration = 0;
    value = gpio_get_level(SENSOR_PIN);
    while(gpio_get_level(SENSOR_PIN) == value && duration < timeout)
        ++duration;
    return duration < timeout;
}

bool readPulse(int& lowDuration, int& highDuration, int timeout)
{
    lowDuration = 0;
    highDuration = 0;
    while(gpio_get_level(SENSOR_PIN) == 0 && lowDuration < timeout)
        ++lowDuration;

    if(lowDuration == timeout)
        return false;

    while(gpio_get_level(SENSOR_PIN) == 1 && highDuration < timeout)
        ++highDuration;

    if(highDuration == timeout)
        return false;

    return true;
}


typedef struct tagPulseInfo
{
    tagPulseInfo() : lowDuration(0), highDuration(0){}
    int lowDuration;
    int highDuration;

    bool isSet()
    {
        return highDuration > 1.1*lowDuration;
    }
} PulseInfo;

//mbedtls_net_usleep(25);
extern "C" void app_main()
{
    sys_init();

    gpio_intr_disable(SENSOR_PIN);
    gpio_wakeup_disable(SENSOR_PIN);
    gpio_set_direction(SENSOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SENSOR_PIN,1);
    sys_msleep(10);

    std::vector<PulseInfo> pulses;
    pulses.reserve(48);

    printf("Init:\n");
    sendSensorInit();
    gpio_set_direction(SENSOR_PIN, GPIO_MODE_INPUT);

    PulseInfo pulse;
    while(readPulse(pulse.lowDuration,pulse.highDuration,10000))
    {
        pulses.push_back(pulse);
        pulse.lowDuration = 0;
        pulse.highDuration = 0;
    }
    pulses.push_back(pulse); //Trailing off

    pulses.erase(pulses.begin());
    printf("Read %i values.\n",pulses.size());
    for(int i = 0; i < pulses.size(); i++)
        printf("Pulse %i:  High value: %i. Low Value: %i\n",i, pulses[i].highDuration,pulses[i].lowDuration);


    int8_t bits[5];
    for(int i = 0; i < 5; i++)
        bits[i] = 0;

    for(int i = 0; i < 5; i++)
        for(int j = 0; j < 8; j++)
            if(pulses[i*8+j].isSet())
                bits[i] |= (1 << (7-j));


    for(int i = 0; i < 5; i++)
    {
        for(int t=128; t>0; t = t/2)
            if(bits[i] & t)
                printf("1");
            else
                printf("0");
            printf("\n");
    }

//    bits[0] &= 0x03;
//    bits[2] &= 0x83;

    // CONVERT AND STORE
    double humidity = (bits[0] * 256 + bits[1]) * 0.1;
    double temperature = ((bits[2] & 0x7F) * 256 + bits[3]) * 0.1;
    if (bits[2] & 0x80)  // negative temperature
    {
        temperature = -temperature;
    }

    // TEST CHECKSUM
    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];

    printf("Humidity: %f. Temp: %f. Checksum: %i. Expected checksum: %i\n",humidity,temperature,bits[4],sum);

//    if (bits[4] != sum)
//    {
//        return DHTLIB_ERROR_CHECKSUM;
//    }

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
