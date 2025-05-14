#ifndef HEARTSENSOR_H
#define HEARTSENSOR_H
#define RATE_SIZE 10
#include <Wire.h>
#include <MAX30105.h>

#include <heartRate.h> 
int heartSensorInit(MAX30105 &particleSensor, int sdaPin, int sclPin);
int getAverageBPM(MAX30105 &particleSensor);
#endif
