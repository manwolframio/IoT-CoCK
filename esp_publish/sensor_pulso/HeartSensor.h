#ifndef HEART_SENSOR_H
#define HEART_SENSOR_H

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

int heartSensorInit(MAX30105 &particleSensor,int sdaPin, int sclPin);
float* heartbeatAcquire(MAX30105 &particleSensor);

#endif
