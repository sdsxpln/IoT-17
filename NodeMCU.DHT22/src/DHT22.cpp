//
// Created by Sergiy Mesropyan on 6/27/17.
//

#include <limits>
#include "lwip/sys.h"
#include "DHT22.h"

DHT22::DHT22(gpio_num_t sensorPin) : m_sensorPin(sensorPin),m_humidity(std::numeric_limits<double>::quiet_NaN()),m_temperature(std::numeric_limits<double>::quiet_NaN())
{
}

bool DHT22::readPulse(int &lowDuration, int &highDuration, int timeout)
{
    lowDuration = 0;
    highDuration = 0;
    while(gpio_get_level(m_sensorPin) == 0 && lowDuration < timeout)
        ++lowDuration;

    if(lowDuration == timeout)
        return false;

    while(gpio_get_level(m_sensorPin) == 1 && highDuration < timeout)
        ++highDuration;

    if(highDuration == timeout)
        return false;

    return true;
}


int DHT22::readSensor()
{
    gpio_intr_disable(m_sensorPin);
    gpio_wakeup_disable(m_sensorPin);
    gpio_set_direction(m_sensorPin, GPIO_MODE_OUTPUT);
    gpio_set_level(m_sensorPin,1);
    ets_delay_us(10000); //10ms high bus level, let sensor charge

    //Send sensor init data transfer
    gpio_set_level(m_sensorPin,0);
    ets_delay_us(2000); //1ms low T-be
    gpio_set_level(m_sensorPin,1);
    ets_delay_us(60); //30 usec high T-go
    gpio_set_level(m_sensorPin,0); //Bring the bus down, wait for sensor

    gpio_set_direction(m_sensorPin, GPIO_MODE_INPUT);

    int lowDuration = 0;
    int highDuration = 0;
    int timeoutDuration = 10000;

    //Read ack
    if(!readPulse(lowDuration,highDuration,timeoutDuration))
        return -1;

    int8_t data[5];
    //Read 8 bytes
    for(int byteIndex = 0; byteIndex < 5; byteIndex++)
    {
        data[byteIndex] = 0;

        for(int bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            lowDuration = 0;
            highDuration = 0;
            if(!readPulse(lowDuration,highDuration,timeoutDuration))
                return -2;

            if(highDuration > lowDuration*1.1)
                data[byteIndex] |= (1 << (7-bitIndex));
        }
    }

    if(readPulse(lowDuration,highDuration,timeoutDuration)) //tail up must time out
        return -3;

    data[0] &= 0x03;
    data[2] &= 0x83;

    //Get data and convert it to humidity & temp
    m_humidity = (data[0] * 256 + data[1]) * 0.1;
    m_temperature = ((data[2] & 0x7F) * 256 + data[3]) * 0.1;
    if (data[2] & 0x80)
        m_temperature = -m_temperature;

    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if(sum != data[4])
        return -4;

    return 0;
}