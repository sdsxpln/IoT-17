    //
//    FILE: dht.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.22
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// see dht.cpp file
//

#ifndef dht_h
#define dht_h

#define DHT_LIB_VERSION "0.1.22"

#define DHTLIB_OK                   0
#define DHTLIB_ERROR_CHECKSUM       -1
#define DHTLIB_ERROR_TIMEOUT        -2
#define DHTLIB_ERROR_CONNECT        -3
#define DHTLIB_ERROR_ACK_L          -4
#define DHTLIB_ERROR_ACK_H          -5

#define DHTLIB_DHT11_WAKEUP         18
#define DHTLIB_DHT_WAKEUP           1

#define DHTLIB_DHT11_LEADING_ZEROS  1
#define DHTLIB_DHT_LEADING_ZEROS    6

#include <stdint.h>
#include <driver/gpio.h>
// max timeout is 100 usec.
// For a 16 Mhz proc 100 usec is 1600 clock cycles
// loops using DHTLIB_TIMEOUT use at least 4 clock cycli
// so 100 us takes max 400 loops
// so by dividing F_CPU by 40000 we "fail" as fast as possible
#ifndef F_CPU
#define DHTLIB_TIMEOUT 1000  // ahould be approx. clock/40000
#else
#define DHTLIB_TIMEOUT (F_CPU/40000)
#endif

#define LOW 0
#define HIGH 1

class dht
{
public:
    dht(gpio_num_t pin);
    // return values:
    // DHTLIB_OK
    // DHTLIB_ERROR_CHECKSUM
    // DHTLIB_ERROR_TIMEOUT
    // DHTLIB_ERROR_CONNECT
    // DHTLIB_ERROR_ACK_L
    // DHTLIB_ERROR_ACK_H
    int8_t read11();
    int8_t read();

    inline int8_t read21() { return read(); };
    inline int8_t read22() { return read(); };
    inline int8_t read33() { return read(); };
    inline int8_t read44() { return read(); };

    double humidity;
    double temperature;

private:
    uint8_t bits[5];  // buffer to receive data
    int8_t _readSensor(uint8_t wakeupDelay, uint8_t leadingZeroBits);
    gpio_num_t m_pin;
};
#endif
//
// END OF FILE
//
