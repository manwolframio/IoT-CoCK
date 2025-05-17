#ifndef MQTT_COCK_H
#define MQTT_COCK_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "time.h"

// Funciones originales
int connect_to_mqtt(MQTTClient &mqtt, const char* client_id, const char* broker_address, int port, WiFiClient& net);
void send_to_mqtt(MQTTClient &mqtt, const char* patient_id, const char* measurement, float value, int zone, int alarm, int priority, int status);

// Funciones TLS añadidas
int connect_to_mqtt_TLS(MQTTClient &mqtt, const char* client_id, const char* broker_address, int port, WiFiClientSecure& net_tls, const char* username, const char* password);
void send_to_mqtt_TLS(MQTTClient &mqtt, const char* patient_id, const char* measurement, float value, int zone, int alarm, int priority, int status);

#endif
