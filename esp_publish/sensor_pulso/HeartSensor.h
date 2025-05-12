#ifndef HEARTSENSOR_H
#define HEARTSENSOR_H

#include <Wire.h>
#include <WiFi.h>
#include <MQTTClient.h>
#include "MAX30105.h"
#include "heartRate.h" // Incluir la librería adecuada

const byte RATE_SIZE = 4; // Definir RATE_SIZE aquí

extern WiFiClient network;
extern MQTTClient mqtt;
extern long lastPrint;

void initializeWiFi();
void initializeMQTT();
int heartSensorInit(MAX30105 &particleSensor, int sdaPin, int sclPin);

#endif
