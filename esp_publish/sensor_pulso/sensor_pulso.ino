#include "HeartSensor.h"

MAX30105 particleSensor;

const int sdaPin = 17; // Pin SDA en ESP32
const int sclPin = 18; // Pin SCL en ESP32

void setup() {
  Serial.println("Inicializando sensor...");
  if(heartSensorInit(particleSensor,sdaPin, sclPin)){
    Serial.println("Sensor inicializado");
  }
  else{
    Serial.println("Sensor no inicializado");
    while(1);
  }
}

void loop() {
  static float* hb;  
  hb = heartbeatAcquire(particleSensor);
  if(hb[0] >= 0) {
    Serial.print("Latido detectado! BPM: ");
    Serial.println(hb[0]);
    Serial.print("Promedio BPM: ");
    Serial.println(hb[1]);
  }
  else{
    Serial.print("Latido NO detectado");
  }

  delay(10); // Pequeño retraso para evitar saturar el procesador
}