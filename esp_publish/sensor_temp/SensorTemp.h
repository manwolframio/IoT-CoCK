#ifndef SENSOR_TEMP_H
#define SENSOR_TEMP_H

#include <Wire.h>
#include "MAX30105.h"

int tempSensorInit(MAX30105 &particleSensor, int sdaPin, int sclPin);
float temperatureAcquire(MAX30105 &particleSensor);

#endif
