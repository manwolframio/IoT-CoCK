#ifndef MQTT_H
#define MQTT_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"

int connect_to_mqtt(MQTTClient &mqtt, const char* client_id, const char* broker_address, int port, WiFiClient& net);
void send_to_mqtt(MQTTClient &mqtt, const char* patient_id,const char* measurement, float value, int zone, int alarm, int priority, int status);

#endif
