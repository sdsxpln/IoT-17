//
//    FILE: dht.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.22
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// 0.1.22 undo delayMicroseconds() for wakeups larger than 8
// 0.1.21 replace delay with delayMicroseconds() + small fix
// 0.1.20 Reduce footprint by using uint8_t as error codes. (thanks to chaveiro)
// 0.1.19 masking error for DHT11 - FIXED (thanks Richard for noticing)
// 0.1.18 version 1.16/17 broke the DHT11 - FIXED
// 0.1.17 replaced micros() with adaptive loopcount
//        removed DHTLIB_INVALID_VALUE
//        added  DHTLIB_ERROR_CONNECT
//        added  DHTLIB_ERROR_ACK_L  DHTLIB_ERROR_ACK_H
// 0.1.16 masking unused bits (less errors); refactored bits[]
// 0.1.15 reduced # micros calls 2->1 in inner loop.
// 0.1.14 replace digital read with faster (~3x) code => more robust low MHz machines.
//
// 0.1.13 fix negative temperature
// 0.1.12 support DHT33 and DHT44 initial version
// 0.1.11 renamed DHTLIB_TIMEOUT
// 0.1.10 optimized faster WAKEUP + TIMEOUT
// 0.1.09 optimize size: timeout check + use of mask
// 0.1.08 added formula for timeout based upon clockspeed
// 0.1.07 added support for DHT21
// 0.1.06 minimize footprint (2012-12-27)
// 0.1.05 fixed negative temperature bug (thanks to Roseman)
// 0.1.04 improved readability of code using DHTLIB_OK in code
// 0.1.03 added error values for temp and humidity when read failed
// 0.1.02 added error codes
// 0.1.01 added support for Arduino 1.0, fixed typos (31/12/2011)
// 0.1.00 by Rob Tillaart (01/04/2011)
//
// inspired by DHT11 library
//
// Released to the public domain
//
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "dht.h"
#include <algorithm>
#include <rom/ets_sys.h>

dht::dht(gpio_num_t pin)
{
  m_pin = pin;

  gpio_config_t io_conf;
  //disable interrupt
  io_conf.intr_type = gpio_int_type_t::GPIO_INTR_DISABLE;
  //set as output mode
  io_conf.mode = gpio_mode_t::GPIO_MODE_OUTPUT;
  //bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = (1 << m_pin);
  //disable pull-down mode
  io_conf.pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
  //disable pull-up mode
  io_conf.pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE;
  //configure GPIO with the given settings
  gpio_config(&io_conf);

}

/////////////////////////////////////////////////////
//
// PUBLIC
//

int8_t dht::read11()
{
    // READ VALUES
    int8_t result = _readSensor(DHTLIB_DHT11_WAKEUP, DHTLIB_DHT11_LEADING_ZEROS);

    // these bits are always zero, masking them reduces errors.
    bits[0] &= 0x7F;
    bits[2] &= 0x7F;

    // CONVERT AND STORE
    humidity    = bits[0];  // bits[1] == 0;
    temperature = bits[2];  // bits[3] == 0;

    // TEST CHECKSUM
    // bits[1] && bits[3] both 0
    uint8_t sum = bits[0] + bits[2];
    if (bits[4] != sum)
    {
        return DHTLIB_ERROR_CHECKSUM;
    }
    return result;
}

int8_t dht::read()
{
    // READ VALUES
    int8_t result = _readSensor(DHTLIB_DHT_WAKEUP, DHTLIB_DHT_LEADING_ZEROS);

    // these bits are always zero, masking them reduces errors.
    bits[0] &= 0x03;
    bits[2] &= 0x83;

    // CONVERT AND STORE
    humidity = (bits[0]*256 + bits[1]) * 0.1;
    temperature = ((bits[2] & 0x7F)*256 + bits[3]) * 0.1;
    if (bits[2] & 0x80)  // negative temperature
    {
        temperature = -temperature;
    }

    // TEST CHECKSUM
    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
    if (bits[4] != sum)
    {
        return DHTLIB_ERROR_CHECKSUM;
    }
    return result;
}

/////////////////////////////////////////////////////
//
// PRIVATE
//

int8_t dht::_readSensor(uint8_t wakeupDelay, uint8_t leadingZeroBits)
{
    // INIT BUFFERVAR TO RECEIVE DATA
    uint8_t mask = 128;
    uint8_t idx = 0;

    uint8_t data = 0;
    uint8_t state = 0;
    uint8_t pstate = 0;
    uint16_t zeroLoop = DHTLIB_TIMEOUT;
    uint16_t delta = 0;

    leadingZeroBits = 40 - leadingZeroBits; // reverse counting...


    // REQUEST SAMPLE
    printf("Requested sample (new2)!\n");
    gpio_set_direction(m_pin,gpio_mode_t::GPIO_MODE_OUTPUT);
    gpio_set_level(m_pin, LOW); // T-be
    ets_delay_us(1000);
    gpio_set_level(m_pin, HIGH); // T-go
    gpio_set_direction(m_pin,gpio_mode_t::GPIO_MODE_INPUT);

    printf("Waiting for ack LOW\n");
    ets_delay_us(10);
    //microseconds system_get_time();
    uint16_t loopCount = 8;
    while(gpio_get_level(m_pin) == HIGH)
    {
        if (--loopCount == 0) return DHTLIB_ERROR_CONNECT;
        ets_delay_us(10);
    }
    printf("Waiting for ack HIGH\n");
    // GET ACKNOWLEDGE or TIMEOUT
    loopCount = 8;
    while(gpio_get_level(m_pin) == LOW)
    {
        if (--loopCount == 0) return DHTLIB_ERROR_ACK_L;
        ets_delay_us(10);
    }

return DHTLIB_ERROR_ACK_L;
    loopCount = DHTLIB_TIMEOUT;

    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (uint8_t i = 40; i != 0; )
    {
        // WAIT FOR FALLING EDGE
        state = gpio_get_level(m_pin);

        if (state == LOW && pstate != LOW)
        {
            if (i > leadingZeroBits) // DHT22 first 6 bits are all zero !!   DHT11 only 1
            {
                zeroLoop = std::min(zeroLoop, loopCount);
                delta = (DHTLIB_TIMEOUT - zeroLoop)/4;
            }
            else if ( loopCount <= (zeroLoop - delta) ) // long -> one
            {
                data |= mask;
            }
            mask >>= 1;
            if (mask == 0)   // next byte
            {
                mask = 128;
                bits[idx] = data;
                idx++;
                data = 0;
            }
            // next bit
            --i;

            // reset timeout flag
            loopCount = DHTLIB_TIMEOUT;
        }
        pstate = state;
        // Check timeout
        if (--loopCount == 0)
        {
            return DHTLIB_ERROR_TIMEOUT;
        }

    }
    gpio_set_direction(m_pin,gpio_mode_t::GPIO_MODE_OUTPUT);
    gpio_set_level(m_pin, HIGH);
    return DHTLIB_OK;
}
//
// END OF FILE
//
