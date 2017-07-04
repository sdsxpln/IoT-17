
#include "Arduino.h"
#include "../lib/DHT/src/DHT22.h"
#include "../lib/AWS_IOT/src/AWS_IOT.h"
#include <WiFi.h>
#include "credentials.h"

char HOST_ADDRESS[]="a13bqieut15g5b.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "arn:aws:iot:us-east-1:659335934027:thing/ESP32_HMDTY_TEMPR_SENSOR";

extern "C" void app_main()
{
    Serial.begin(115200);
    delay(1000);
    DHT22 sensor(GPIO_NUM_26);
    delay(1000);
    int errorCode = 0;
    for(int i = 0; i < 20; i++)
        if((errorCode = sensor.readSensor()) != 0)
            printf("Read sensor error: %i\n",errorCode);
        else
            break;

    printf("Read sensor success. Temp: %f, Humidity: %f\n",sensor.temperature(),sensor.humidity());

    Serial.printf("\nAttempting to connect ro wifi: %s password: %s\n",WIFI_SSID,WIFI_PASSWORD);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if(WiFi.waitForConnectResult() == WL_CONNECTED)
    {
        Serial.println("WiFi is connected!n");
    } else
    {
        Serial.printf("WiFi connection error: %i\n",WiFi.status());
        return;
    }

    AWS_IOT awsIoT;   // AWS_IOT instance
    if(awsIoT.connect(HOST_ADDRESS,CLIENT_ID,rootCA,deviceCert,devicePrivateKey)== 0) // Connect to AWS using Host Address and Cliend ID
    {
        Serial.println("Connected to AWS\n");
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address\n");
    }

    delay(1000);
    char szBuff[255];

    sprintf(szBuff,"%f", sensor.temperature()); // Create the payload for publishing
    int publishErrorCode = awsIoT.publish("temperature",szBuff);
    if(publishErrorCode == 0)   // Publish the message(Temp and humidity)
    {
        Serial.print("Publish temperature Success\n");
    }
    else
    {
        Serial.printf("Publish failed. Error code: %i\n",publishErrorCode);
    }

    sprintf(szBuff,"%f", sensor.humidity()); // Create the payload for publishing
    publishErrorCode = awsIoT.publish("humidity",szBuff);
    if(publishErrorCode == 0)   // Publish the message(Temp and humidity)
    {
        Serial.print("Publish temperature Success\n");
    }
    else
    {
        Serial.printf("Publish failed. Error code: %i\n",publishErrorCode);
    }

    esp_deep_sleep(1000000LL * 10); //10s deep sleep
}