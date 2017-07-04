//
// Created by Sergiy Mesropyan on 6/27/17.
//

#ifndef NODEMCU_DHT22_DHT22_H
#define NODEMCU_DHT22_DHT22_H

#include "driver/gpio.h"
class DHT22
{

public:
    DHT22(gpio_num_t sensorPin);

    int    readSensor();
    double humidity() const { return m_humidity; }
    double temperature() const { return m_temperature; }

protected:
    bool readPulse(int& lowDuration, int& highDuration, int timeout);
private:
    gpio_num_t  m_sensorPin;
    double      m_humidity;
    double      m_temperature;
};


#endif //NODEMCU_DHT22_DHT22_H
