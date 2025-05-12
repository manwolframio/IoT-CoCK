#include <Wire.h>
#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "HeartSensor.h"
#include "mqtt.h"

byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
MAX30105 particleSensor;
const int sdaPin = 17; // Pin SDA en ESP32
const int sclPin = 18; // Pin SCL en ESP32

void setup() {
  Serial.begin(115200);
  initializeWiFi();
  initializeMQTT();
  heartSensorInit(particleSensor, sdaPin, sclPin);
}

void loop() {
  mqtt.loop();

  long irValue = particleSensor.getIR();
    if (checkForBeat(irValue)) {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        float beatsPerMinute = 60 / (delta / 1000.0);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= RATE_SIZE;
        }
    }

    int beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
    }
    beatAvg /= RATE_SIZE;

  if (millis() - lastPrint >= 2000) {
    Serial.print("Promedio BPM: ");
    Serial.println(beatAvg);

    publishBpmToMqtt(beatAvg);
    lastPrint = millis();
  }

  delay(10);
}
