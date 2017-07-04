#!/usr/bin/env bash

rm -f credentials.h
echo "#ifndef NODEMCU_ARDUINO_CREDENTIALS_H" >> credentials.h
echo "#define NODEMCU_ARDUINO_CREDENTIALS_H" >> credentials.h

echo ""  >> credentials.h
echo 'const char* WIFI_SSID ="GSGSJS762TS";' >> credentials.h
echo 'const char* WIFI_PASSWORD ="love4fun";' >> credentials.h
echo ""  >> credentials.h
 echo "const char *rootCA =" >> credentials.h
cat ../certs/root-ca.crt | tr -d "\r" | awk '{ print "\""$0"\\r\\n\""}' >> credentials.h
echo ";" >> credentials.h
echo "const char *devicePrivateKey =" >> credentials.h
cat ../certs/private.pem.key | tr -d "\r" | awk '{ print "\""$0"\\r\\n\""}' >> credentials.h
echo ";" >> credentials.h
echo "const char *deviceCert =" >> credentials.h
cat ../certs/certificate.pem.crt | tr -d "\r" | awk '{ print "\""$0"\\r\\n\""}' >> credentials.h
echo ";" >> credentials.h

echo "#endif //NODEMCU_ARDUINO_CREDENTIALS_H" >> credentials.h
