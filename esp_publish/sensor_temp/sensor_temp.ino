#include "SensorTemp.h"

MAX30105 particleSensor; // Crear una instancia global del sensor

const int sdaPin = 17; // Pin SDA del ESP32
const int sclPin = 18; // Pin SCL del ESP32

void setup() {
    Serial.begin(115200); // Inicializar comunicación serie
    delay(1000); // Pequeña pausa para estabilidad al iniciar

    // Inicializar el sensor y verificar si fue exitoso
    if (tempSensorInit(particleSensor, sdaPin, sclPin) == -1) {
        Serial.println("Fallo en la inicialización del sensor. Verifica conexiones.");
        while (1); // Detener ejecución si hay un fallo
    }

    Serial.println("Sensor MAX30105 inicializado correctamente.");
}

void loop() {
    // Obtener temperatura en Celsius
    float temperature;
    
    temperature = temperatureAcquire(particleSensor);
    
    Serial.print("Temperatura (Celsius): ");
    Serial.print(temperature, 2);
    delay(1000); // Esperar 1 segundo antes de la próxima medición
}
